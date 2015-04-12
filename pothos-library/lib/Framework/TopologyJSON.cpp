// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/TopologyImpl.hpp>
#include "Framework/TopologyImpl.hpp"
#include <Pothos/Proxy.hpp>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>
#include <Poco/File.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <future>
#include <map>

/***********************************************************************
 * String/file parser - make JSON object from string
 **********************************************************************/
static Poco::JSON::Object::Ptr parseJSONStr(const std::string &json)
{
    //determine markup string or file path
    bool isPath = false;
    try {isPath = Poco::File(json).exists();}
    catch (...){}

    //parse the json string/file to a JSON object
    Poco::JSON::Parser p;
    if (isPath)
    {
        std::ifstream ifs(json);
        p.parse(ifs);
    }
    else
    {
        p.parse(json);
    }
    return p.getHandler()->asVar().extract<Poco::JSON::Object::Ptr>();
}

/***********************************************************************
 * evaluate an args array (calls and constructors)
 **********************************************************************/
static std::vector<Pothos::Proxy> evalArgsArray(
    const Pothos::Proxy &evaluator,
    const Poco::JSON::Array::Ptr &argsArray,
    const size_t offset = 0)
{
    std::vector<Pothos::Proxy> args;
    if (argsArray) for (size_t i = offset; i < argsArray->size(); i++)
    {
        auto arg = argsArray->get(i).toString();
        if (argsArray->get(i).isString()) arg = "\""+arg+"\"";
        const auto obj = evaluator.call<Pothos::Object>("eval", arg);
        args.push_back(evaluator.getEnvironment()->convertObjectToProxy(obj));
    }
    return args;
}

/***********************************************************************
 * block factory - make blocks from JSON object
 **********************************************************************/
static Pothos::Proxy makeBlock(
    const Pothos::Proxy &registry,
    const Pothos::Proxy &evaluator,
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
    auto block = registry.getHandle()->call(path, ctorArgs.data(), ctorArgs.size());

    //make the calls
    Poco::JSON::Array::Ptr callsArray;
    if (blockObj->isArray("calls")) callsArray = blockObj->getArray("calls");
    if (callsArray) for (size_t i = 0; i < callsArray->size(); i++)
    {
        const auto callArray = callsArray->getArray(i);
        auto name = callArray->getElement<std::string>(0);
        const auto callArgs = evalArgsArray(evaluator, callArray, 1/*offset*/);
        block.getHandle()->call(name, callArgs.data(), callArgs.size());
    }

    return block;
}

/***********************************************************************
 * make topology from JSON string - implementation
 **********************************************************************/
std::shared_ptr<Pothos::Topology> Pothos::Topology::make(const std::string &json)
{
    //parse the json string/file to a JSON object
    const auto topObj = parseJSONStr(json);

    //create the proxy environment (local) and the registry
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");
    auto evaluator = env->findProxy("Pothos/Util/EvalEnvironment").callProxy("make");

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

/***********************************************************************
 * create JSON stats object
 **********************************************************************/
static std::pair<std::string, Poco::JSON::Object::Ptr> queryWorkStats(const Pothos::Proxy &block)
{
    auto actor = block.callProxy("get:_actor");
    auto workStats = actor.call<Poco::JSON::Object::Ptr>("queryWorkStats");
    return std::make_pair(block.call<std::string>("uid"), workStats);
}

std::string Pothos::Topology::queryJSONStats(void)
{
    Poco::JSON::Object::Ptr stats(new Poco::JSON::Object());

    //query each block's work stats and key it with the UID
    std::vector<std::shared_future<std::pair<std::string, Poco::JSON::Object::Ptr>>> results;
    for (const auto &block : getObjSetFromFlowList(_impl->flows))
    {
        results.push_back(std::async(std::launch::async, queryWorkStats, block));
    }

    //wait on the futures and record to the object
    for (const auto &result : results)
    {
        const auto workStats = result.get();
        stats->set(workStats.first, workStats.second);
    }

    //return the string-formatted result
    std::stringstream ss; stats->stringify(ss, 4);
    return ss.str();
}

/***********************************************************************
 * dump to structured view
 **********************************************************************/

std::string Pothos::Topology::dumpJSON(const std::string &request)
{
    //extract input request
    Poco::JSON::Parser p; p.parse(request.empty()?"{}":request);
    auto configObj = p.getHandler()->asVar().extract<Poco::JSON::Object::Ptr>();
    const auto modeConfig = configObj->optValue<std::string>("mode", "flat");
    const auto renderedConfig = configObj->optValue<bool>("rendered", false);

    Poco::JSON::Object::Ptr topObj(new Poco::JSON::Object());

    //create blocks map
    Poco::JSON::Object::Ptr blocksObj(new Poco::JSON::Object());
    topObj->set("blocks", blocksObj);
    for (const auto &block : getObjSetFromFlowList(_impl->flows))
    {
        Poco::JSON::Object::Ptr blockObj(new Poco::JSON::Object());
        blocksObj->set(block.call<std::string>("uid"), blockObj);
        blockObj->set("name", block.call<std::string>("getName"));

        Poco::JSON::Array::Ptr inputsArray(new Poco::JSON::Array());
        for (const auto &portInfo : block.call<std::vector<PortInfo>>("inputPortInfo"))
        {
            Poco::JSON::Object::Ptr infoObj(new Poco::JSON::Object());
            inputsArray->add(infoObj);
            infoObj->set("name", portInfo.name);
            infoObj->set("dtype", portInfo.dtype.toString());
            if (portInfo.isSigSlot) infoObj->set("type", "slot");
        }
        if (inputsArray->size() > 0) blockObj->set("inputs", inputsArray);

        Poco::JSON::Array::Ptr outputsArray(new Poco::JSON::Array());
        for (const auto &portInfo : block.call<std::vector<PortInfo>>("outputPortInfo"))
        {
            Poco::JSON::Object::Ptr infoObj(new Poco::JSON::Object());
            outputsArray->add(infoObj);
            infoObj->set("name", portInfo.name);
            infoObj->set("dtype", portInfo.dtype.toString());
            if (portInfo.isSigSlot) infoObj->set("type", "signal");
        }
        if (outputsArray->size() > 0) blockObj->set("outputs", outputsArray);
    }

    //create connections list
    Poco::JSON::Array::Ptr connsArray(new Poco::JSON::Array());
    topObj->set("connections", connsArray);
    for (const auto &flow : _impl->flows)
    {
        Poco::JSON::Array::Ptr connArray(new Poco::JSON::Array());
        connsArray->add(connArray);
        connArray->add(flow.src.uid);
        connArray->add(flow.src.name);
        connArray->add(flow.dst.uid);
        connArray->add(flow.dst.name);
    }

    //return the string-formatted result
    std::stringstream ss; topObj->stringify(ss, 4);
    return ss.str();
}
