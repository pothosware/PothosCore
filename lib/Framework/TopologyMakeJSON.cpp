// Copyright (c) 2014-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/TopologyImpl.hpp>
#include <Pothos/Util/EvalEnvironment.hpp>
#include <Pothos/Proxy.hpp>
#include <Poco/Format.h>
#include <map>
#include <json.hpp>

using json = nlohmann::json;

/***********************************************************************
 * evaluation helpers
 **********************************************************************/
static Pothos::Object evalExpression(
    Pothos::Util::EvalEnvironment &evaluator,
    const json &arg)
{
    if (arg.is_string())
    {
        try
        {
            //it could be an expression
            return evaluator.eval(arg.get<std::string>());
        }
        catch (...)
        {
            //fail, so treat it as a literal string
            return Pothos::Object(arg.get<std::string>());
        }
    }

    //otherwise run the evaluator as normal
    return evaluator.eval(arg.dump());
}

static std::vector<Pothos::Proxy> evalArgsArray(
    Pothos::Util::EvalEnvironment &evaluator,
    const json &argsArray,
    const size_t offset = 0)
{
    std::vector<Pothos::Object> args;
    for (size_t i = offset; i < argsArray.size(); i++)
    {
        args.push_back(evalExpression(evaluator, argsArray.at(i)));
    }
    return Pothos::Object(args);
}

typedef std::vector<std::pair<std::string, json>> OrderedVarMap;

static OrderedVarMap extractVariableMap(const json &obj, const std::string &key, const std::string &what)
{
    OrderedVarMap result;
    size_t i = 0;
    if (obj.count(key) == 0) return result;
    const auto &varMap = obj[key];

    //ordered array of variables format
    if (varMap.is_array()) for (const auto &varObj : varMap)
    {
        const std::string &what_i = what + "[" + std::to_string(i++) + "]";
        if (not varObj.is_object()) throw Pothos::DataFormatException(what_i+" not an object");
        if (varObj.count("name") == 0) throw Pothos::DataFormatException(what_i+" missing 'name' field");
        if (varObj.count("value") == 0) throw Pothos::DataFormatException(what_i+" missing 'value' field");
        result.emplace_back(varObj["name"].get<std::string>(), varObj["value"]);
    }

    //support unordered maps
    else if (varMap.is_object())
    {
        for (auto it = varMap.begin(); it != varMap.end(); ++it)
        {
            result.emplace_back(it.key(), it.value());
        }
    }

    //otherwise unknown
    else throw Pothos::DataFormatException("variables must be an array or object");

    return result;
}

/***********************************************************************
 * block factory - make blocks from JSON object
 **********************************************************************/
static Pothos::Proxy makeBlock(
    const Pothos::Proxy &registry,
    const OrderedVarMap &globals,
    const json &blockObj)
{
    const std::string id = blockObj["id"];

    if (blockObj.count("path") == 0) throw Pothos::DataFormatException(
        "Pothos::Topology::make()", "blocks["+id+"] missing 'path' field");
    const std::string path = blockObj["path"];

    //parse the local variables
    auto locals = extractVariableMap(blockObj, "locals", id+".locals");

    //prepend globals into the local variable space
    locals.insert(locals.begin(), globals.begin(), globals.end());

    //load the evaluator with variables
    Pothos::Util::EvalEnvironment evaluator;
    for (const auto &pair : locals)
    {
        const auto result = evalExpression(evaluator, pair.second);
        evaluator.registerConstantObj(pair.first, result);
    }

    //load up the constructor args
    const auto &argsArray = blockObj.value("args", json::array());
    const auto ctorArgs = evalArgsArray(evaluator, argsArray);

    //create the block
    Pothos::Proxy block;
    try
    {
        block = registry.getHandle()->call(path, ctorArgs.data(), ctorArgs.size());
    }
    catch (const Pothos::Exception &ex)
    {
        std::string argsStr;
        for (const auto &arg : ctorArgs) argsStr += arg.toString() + (argsStr.empty()?"":", ");
        throw Pothos::RuntimeException(Poco::format("%s = %s(%s)", id, path, argsStr), ex);
    }

    //make the calls
    const auto &callsArray = blockObj.value("calls", json::array());
    for (const auto &callArray : callsArray)
    {
        auto name = callArray[0].get<std::string>();
        const auto callArgs = evalArgsArray(evaluator, callArray, 1/*offset*/);
        try
        {
            block.getHandle()->call(name, callArgs.data(), callArgs.size());
        }
        catch (const Pothos::Exception &ex)
        {
            std::string argsStr;
            for (const auto &arg : callArgs) argsStr += arg.toString() + (argsStr.empty()?"":", ");
            throw Pothos::RuntimeException(Poco::format("%s.%s(%s)", id, name, argsStr), ex);
        }
    }

    return block;
}

/***********************************************************************
 * make topology from JSON string - implementation
 **********************************************************************/
std::shared_ptr<Pothos::Topology> Pothos::Topology::make(const std::string &jsonStr)
{
    //parse the json formatted string into a JSON object
    json topObj;
    try
    {
        topObj = json::parse(jsonStr);
    }
    catch (const std::exception &ex)
    {
        throw Pothos::DataFormatException("Pothos::Topology::make()", ex.what());
    }

    //create the proxy environment (local) and the registry
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");
    auto evaluator = Pothos::Util::EvalEnvironment();

    //create thread pools
    std::map<std::string, Pothos::Proxy> threadPools;
    const auto &threadPoolObj = topObj.value("threadPools", json::object());
    for (auto it = threadPoolObj.begin(); it != threadPoolObj.end(); ++it)
    {
        Pothos::ThreadPoolArgs args(it.value().dump());
        threadPools[it.key()] = env->findProxy("Pothos/ThreadPool")(args);
    }

    //parse global variables
    const auto globals = extractVariableMap(topObj, "globals", "globals");

    //create the topology and add it to the blocks
    //the IDs 'self', 'this', and '' can be used
    std::map<std::string, Pothos::Proxy> blocks;
    auto topology = Pothos::Topology::make();
    blocks["self"] = env->makeProxy(topology);
    blocks["this"] = env->makeProxy(topology);
    blocks[""] = env->makeProxy(topology);

    //create the blocks
    const auto &blockArray = topObj.value("blocks", json::array());
    for (size_t i = 0; i < blockArray.size(); i++)
    {
        const auto &blockObj = blockArray.at(i);
        if (not blockObj.is_object()) throw Pothos::DataFormatException(
            "Pothos::Topology::make()", "blocks["+std::to_string(i)+"] must be an object");
        if (not blockObj.count("id")) throw Pothos::DataFormatException(
            "Pothos::Topology::make()", "blocks["+std::to_string(i)+"] missing 'id' field");
        const auto id = blockObj["id"].get<std::string>();
        blocks[id] = makeBlock(registry, globals, blockObj);

        //set the thread pool
        const auto threadPoolName = blockObj.value<std::string>("threadPool", "");
        auto threadPoolIt = threadPools.find(threadPoolName);
        if (threadPoolIt != threadPools.end()) blocks[id].callVoid("setThreadPool", threadPoolIt->second);
        else if (not threadPoolName.empty()) throw Pothos::DataFormatException(
            "Pothos::Topology::make()", "blocks["+id+"] unknown threadPool = " + threadPoolName);
    }

    //create the topology and connect the blocks
    const auto &connArray = topObj.value("connections", json::array());
    for (size_t i = 0; i < connArray.size(); i++)
    {
        const auto &connArgs = connArray.at(i);
        if (not connArgs.is_array()) throw Pothos::DataFormatException(
            "Pothos::Topology::make()", "connections["+std::to_string(i)+"] must be an array");
        if (connArgs.size() != 4) throw Pothos::DataFormatException(
            "Pothos::Topology::make()", "connections["+std::to_string(i)+"] must be size 4");

        //get string value or dump the value to a string
        auto optStr = [](const json &v) -> std::string
        {
            return v.is_string()?v.get<std::string>():v.dump();
        };

        //extract connection arg fields
        const std::string srcId = connArgs.at(0);
        const auto srcPort = optStr(connArgs.at(1));
        const std::string dstId = connArgs.at(2);
        const auto dstPort = optStr(connArgs.at(3));

        //check that the block IDs exist
        if (blocks.count(srcId) == 0) throw Pothos::DataFormatException(
            "Pothos::Topology::make()", "connections["+std::to_string(i)+"] no such ID: " + srcId);
        if (blocks.count(dstId) == 0) throw Pothos::DataFormatException(
            "Pothos::Topology::make()", "connections["+std::to_string(i)+"] no such ID: " + dstId);

        //make the connection
        topology->connect(blocks.at(srcId), srcPort, blocks.at(dstId), dstPort);
    }

    return topology;
}
