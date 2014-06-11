// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "GraphEditor/ExecutionEngine.hpp"
#include <GraphObjects/GraphBlock.hpp>
#include <GraphObjects/GraphBreaker.hpp>
#include <GraphObjects/GraphConnection.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Object/Containers.hpp>
#include <QString>
#include <map>
#include <tuple>
#include <iostream>

//TODO temporary - replace w/ JIT
static Pothos::Object parseOne(const std::string &val)
{
    if (val.size() >= 2 and val.front() == '"' and val.back() == '"') return Pothos::Object(val.substr(1, val.size()-2));
    if (val == "true") return Pothos::Object(true);
    if (val == "false") return Pothos::Object(false);
    try {return Pothos::Object(Poco::NumberParser::parseUnsigned64(val));}
    catch (const Poco::SyntaxException &){}
    try {return Pothos::Object(Poco::NumberParser::parse64(val));}
    catch (const Poco::SyntaxException &){}
    try {return Pothos::Object(Poco::NumberParser::parseHex64(val));}
    catch (const Poco::SyntaxException &){}
    try {return Pothos::Object(Poco::NumberParser::parseFloat(val));}
    catch (const Poco::SyntaxException &){}
    try {return Pothos::Object(Poco::NumberParser::parse(val));}
    catch (const Poco::SyntaxException &){}
    std::cerr << ("cant parse " + val) << std::endl;
    throw Pothos::Exception("GraphBlock::update()", "cant parse " + val);
}

Pothos::Proxy graphBlockToProxyBlock(GraphBlock *block)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");
    auto blockDesc = block->getBlockDesc();

    //load up the constructor args
    std::vector<Pothos::Proxy> ctorArgs;
    for (auto arg : *blockDesc->getArray("args"))
    {
        const auto propKey = arg.extract<std::string>();
        const auto propVal = block->getPropertyValue(QString::fromStdString(propKey));
        const auto obj = parseOne(propVal.toStdString());
        ctorArgs.push_back(env->convertObjectToProxy(obj));
    }

    //create the block
    auto proxyBlock = registry.getHandle()->call(block->getBlockDescPath(), ctorArgs.data(), ctorArgs.size());
    proxyBlock.call("setName", block->getId().toStdString());

    //make the calls
    for (auto call : *blockDesc->getArray("calls"))
    {
        const auto callObj = call.extract<Poco::JSON::Object::Ptr>();
        const auto callName = callObj->get("name").extract<std::string>();
        std::vector<Pothos::Proxy> callArgs;
        for (auto arg : *callObj->getArray("args"))
        {
            const auto propKey = arg.extract<std::string>();
            const auto propVal = block->getPropertyValue(QString::fromStdString(propKey));
            const auto obj = parseOne(propVal.toStdString());
            callArgs.push_back(env->convertObjectToProxy(obj));
        }
        proxyBlock.getHandle()->call(callName, callArgs.data(), callArgs.size());
    }

    return proxyBlock;
}

/*!
 * Given an input endpoint, discover all of the "resolved" input endpoints by traversing breakers of the same node name.
 */
static std::vector<GraphConnectionEndpoint> traverseInputEps(const GraphConnectionEndpoint &inputEp, const GraphObjectList &graphObjects)
{
    std::vector<GraphConnectionEndpoint> inputEndpoints;
    auto inputBlock = dynamic_cast<GraphBlock *>(inputEp.getObj().data());
    auto inputBreaker = dynamic_cast<GraphBreaker *>(inputEp.getObj().data());

    if (inputBlock != nullptr)
    {
        inputEndpoints.push_back(inputEp);
    }

    if (inputBreaker != nullptr)
    {
        auto nodeName = inputBreaker->getNodeName();
        for (auto graphObject : graphObjects)
        {
            auto breaker = dynamic_cast<GraphBreaker *>(graphObject);
            if (breaker == nullptr) continue;
            if (breaker->getNodeName() != nodeName) continue;
            if (breaker == inputBreaker) continue;
            //follow all connections from this breaker to an input
            //this is the recursive part
            for (auto graphSubObject : graphObjects)
            {
                auto connection = dynamic_cast<GraphConnection *>(graphSubObject);
                if (connection == nullptr) continue;
                if (connection->getOutputEndpoint().getObj() != breaker) continue;
                for (const auto &subEp : traverseInputEps(connection->getInputEndpoint(), graphObjects))
                {
                    inputEndpoints.push_back(subEp);
                }
            }
        }
    }

    return inputEndpoints;
}

static std::vector<std::tuple<QString, QString, QString, QString>> getConnectionTuples(const GraphObjectList &graphObjects)
{
    std::vector<std::tuple<QString, QString, QString, QString>> connections;
    for (auto graphObject : graphObjects)
    {
        auto connection = dynamic_cast<GraphConnection *>(graphObject);
        if (connection == nullptr) continue;
        auto outputEp = connection->getOutputEndpoint();
        auto inputEp = connection->getInputEndpoint();

        //auto outputBlock = dynamic_cast<GraphBlock *>(outputEp.getObj().data());
        auto outputBreaker = dynamic_cast<GraphBreaker *>(outputEp.getObj().data());
        //auto inputBlock = dynamic_cast<GraphBlock *>(inputEp.getObj().data());
        //auto inputBreaker = dynamic_cast<GraphBreaker *>(inputEp.getObj().data());

        //ignore connections from output breakers
        //we will come back to them from the block to breaker to block path
        if (outputBreaker != nullptr) continue;

        for (const auto &subEp : traverseInputEps(inputEp, graphObjects))
        {
            connections.emplace_back(
                outputEp.getObj()->getId(), outputEp.getKey().id,
                subEp.getObj()->getId(), subEp.getKey().id);
        }

    }
    return connections;
}

ExecutionEngine::~ExecutionEngine(void)
{
    return;
}

class ExecutionEngineImpl : public ExecutionEngine
{
public:
    ExecutionEngineImpl(void)
    {
        return;
    }

    void update(const GraphObjectList &graphObjects)
    {
        std::cout << "update called with " << graphObjects.size() << "objects\n";
        _graphObjects = graphObjects;
    }

    void activate(void)
    {
        std::cout << "activate called \n";
        this->updateBlocks(_graphObjects);
        std::cout << "num blocks " << _idToBlock.size() << std::endl;
        _topology.disconnectAll();
        for (const auto &t : getConnectionTuples(_graphObjects))
        {
            std::cout << "make connection \n";
            _topology.connect(
                _idToBlock[std::get<0>(t)],
                std::get<1>(t).toStdString(),
                _idToBlock[std::get<2>(t)],
                std::get<3>(t).toStdString());
        }
        _topology.commit();
    }

    void deactivate(void)
    {
        _topology.disconnectAll();
        _topology.commit();
    }

    std::string toDotMarkup(void)
    {
        return _topology.toDotMarkup();
    }

private:

    void updateBlocks(const GraphObjectList &graphObjects)
    {
        //TODO update/change blocks only when when there are changes - do the minimum amount of work
        std::map<QString, Pothos::Proxy> newBlocks;
        for (auto graphObject : graphObjects)
        {
            auto block = dynamic_cast<GraphBlock *>(graphObject);
            if (block == nullptr) continue;
            newBlocks[block->getId()] = graphBlockToProxyBlock(block);
        }
        _idToBlock = newBlocks;
    }

    std::map<QString, Pothos::Proxy> _idToBlock;

    QObject *_graph;
    Pothos::Topology _topology;
    GraphObjectList _graphObjects;
};

ExecutionEngine *ExecutionEngine::make(void)
{
    return new ExecutionEngineImpl();
}
