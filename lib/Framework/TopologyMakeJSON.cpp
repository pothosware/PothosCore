// Copyright (c) 2014-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/TopologyImpl.hpp>
#include <Pothos/Util/EvalEnvironment.hpp>
#include <Pothos/Proxy.hpp>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Format.h>
#include <sstream>
#include <iostream>
#include <map>

/***********************************************************************
 * evaluation helpers
 **********************************************************************/
static Pothos::Object evalExpression(
    Pothos::Util::EvalEnvironment &evaluator,
    const Poco::Dynamic::Var &arg)
{
    if (arg.isString())
    {
        try
        {
            //it could be an expression
            return evaluator.eval(arg.toString());
        }
        catch (...)
        {
            //fail, so treat it as a literal string
            return Pothos::Object(arg.toString());
        }
    }

    //otherwise run the evaluator as normal
    return evaluator.eval(arg.toString());
}

static std::vector<Pothos::Proxy> evalArgsArray(
    Pothos::Util::EvalEnvironment &evaluator,
    const Poco::JSON::Array::Ptr &argsArray,
    const size_t offset = 0)
{
    std::vector<Pothos::Object> args;
    if (argsArray) for (size_t i = offset; i < argsArray->size(); i++)
    {
        args.push_back(evalExpression(evaluator, argsArray->get(i)));
    }
    return Pothos::Object(args).convert<std::vector<Pothos::Proxy>>();
}

/***********************************************************************
 * block factory - make blocks from JSON object
 **********************************************************************/
static Pothos::Proxy makeBlock(
    const Pothos::Proxy &registry,
    Pothos::Util::EvalEnvironment &evaluator,
    const Poco::JSON::Object::Ptr &blockObj)
{
    const auto id = blockObj->getValue<std::string>("id");

    if (not blockObj->has("path")) throw Pothos::DataFormatException(
        "Pothos::Topology::make()", "blocks["+id+"] missing 'path' field");
    const auto path = blockObj->getValue<std::string>("path");

    //load up the constructor args
    Poco::JSON::Array::Ptr argsArray;
    if (blockObj->isArray("args")) argsArray = blockObj->getArray("args");
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
    Poco::JSON::Array::Ptr callsArray;
    if (blockObj->isArray("calls")) callsArray = blockObj->getArray("calls");
    if (callsArray) for (size_t i = 0; i < callsArray->size(); i++)
    {
        const auto callArray = callsArray->getArray(i);
        auto name = callArray->getElement<std::string>(0);
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
std::shared_ptr<Pothos::Topology> Pothos::Topology::make(const std::string &json)
{
    //parse the json formatted string into a JSON object
    Poco::JSON::Object::Ptr topObj;
    try
    {
        const auto result = Poco::JSON::Parser().parse(json);
        topObj = result.extract<Poco::JSON::Object::Ptr>();
    }
    catch (const Poco::Exception &ex)
    {
        throw Pothos::DataFormatException("Pothos::Topology::make()", ex.message());
    }

    //create the proxy environment (local) and the registry
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");
    auto evaluator = Pothos::Util::EvalEnvironment();

    //create thread pools
    std::map<std::string, Pothos::Proxy> threadPools;
    Poco::JSON::Object::Ptr threadPoolObj;
    if (topObj->isObject("threadPools")) threadPoolObj = topObj->getObject("threadPools");
    std::vector<std::string> threadPoolNames;
    if (threadPoolObj) threadPoolObj->getNames(threadPoolNames);
    for (const auto &name : threadPoolNames)
    {
        std::stringstream ss;
        threadPoolObj->getObject(name)->stringify(ss);
        Pothos::ThreadPoolArgs args(ss.str());
        threadPools[name] = env->findProxy("Pothos/ThreadPool").callProxy("new", args);
    }

    //register global variables
    Poco::JSON::Array::Ptr globalsArray;
    if (topObj->isArray("globals")) globalsArray = topObj->getArray("globals");
    if (globalsArray) for (size_t i = 0; i < globalsArray->size(); i++)
    {
        if (not globalsArray->isObject(i)) throw Pothos::DataFormatException(
            "Pothos::Topology::make()", "globals["+std::to_string(i)+"] must be an object");
        const auto &globalVarObj = globalsArray->getObject(i);
        if (not globalVarObj->has("name")) throw Pothos::DataFormatException(
            "Pothos::Topology::make()", "globals["+std::to_string(i)+"] missing 'name' field");
        const auto name = globalVarObj->getValue<std::string>("name");
        if (not globalVarObj->has("value")) throw Pothos::DataFormatException(
            "Pothos::Topology::make()", "globals["+std::to_string(i)+"] missing 'value' field");

        //evaluate and store the result into the evaluator's constants
        const auto value = evalExpression(evaluator, globalVarObj->get("value"));
        evaluator.registerConstantObj(name, value);
    }

    //create the topology and add it to the blocks
    //the IDs 'self', 'this', and '' can be used
    std::map<std::string, Pothos::Proxy> blocks;
    auto topology = Pothos::Topology::make();
    blocks["self"] = env->makeProxy(topology);
    blocks["this"] = env->makeProxy(topology);
    blocks[""] = env->makeProxy(topology);

    //create the blocks
    Poco::JSON::Array::Ptr blockArray;
    if (topObj->isArray("blocks")) blockArray = topObj->getArray("blocks");
    if (blockArray) for (size_t i = 0; i < blockArray->size(); i++)
    {
        if (not blockArray->isObject(i)) throw Pothos::DataFormatException(
            "Pothos::Topology::make()", "blocks["+std::to_string(i)+"] must be an object");
        const auto &blockObj = blockArray->getObject(i);
        if (not blockObj->has("id")) throw Pothos::DataFormatException(
            "Pothos::Topology::make()", "blocks["+std::to_string(i)+"] missing 'id' field");
        const auto id = blockObj->getValue<std::string>("id");
        blocks[id] = makeBlock(registry, evaluator, blockObj);

        //set the thread pool
        const auto threadPoolName = blockObj->optValue<std::string>("threadPool", "default");
        auto threadPoolIt = threadPools.find(threadPoolName);
        if (threadPoolIt != threadPools.end()) blocks[id].callVoid("setThreadPool", threadPoolIt->second);
        else if (threadPoolName != "default") throw Pothos::DataFormatException(
            "Pothos::Topology::make()", "blocks["+id+"] unknown threadPool = " + threadPoolName);
    }

    //create the topology and connect the blocks
    Poco::JSON::Array::Ptr connArray;
    if (topObj->isArray("connections")) connArray = topObj->getArray("connections");
    if (connArray) for (size_t i = 0; i < connArray->size(); i++)
    {
        if (not connArray->isArray(i)) throw Pothos::DataFormatException(
            "Pothos::Topology::make()", "connections["+std::to_string(i)+"] must be an array");
        const auto &connArgs = connArray->getArray(i);
        if (connArgs->size() != 4) throw Pothos::DataFormatException(
            "Pothos::Topology::make()", "connections["+std::to_string(i)+"] must be size 4");

        //extract connection arg fields
        const auto srcId = connArgs->getElement<std::string>(0);
        const auto srcPort = connArgs->get(1).toString();
        const auto dstId = connArgs->getElement<std::string>(2);
        const auto dstPort = connArgs->get(3).toString();

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
