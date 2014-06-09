// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGui.hpp"
#include <GraphObjects/GraphBlock.hpp>
#include <GraphObjects/GraphBreaker.hpp>
#include <GraphObjects/GraphConnection.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Object/Containers.hpp>
#include <QString>
#include <map>
#include <tuple>

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


static std::vector<std::tuple<QString, QString, QString, QString>> getConnectionTuples(const GraphObjectList &graphObjects)
{
    std::tuple<QString, QString, QString, QString> connections;
    for (auto graphObject : graphObjects)
    {
        auto connection = dynamic_cast<GraphBlock *>(graphConnection);
        if (connection == nullptr) continue;
        auto outputEp = connection->getOutputEndpoint();
        auto inputEp = connection->getInputEndpoint();

        

    }
    return connections;
}

class ExecutionEngine
{
public:
    ExecutionEngine(void)
    {
        
    };

    void update(const GraphObjectList &graphObjects)
    {
        this->updateBlocks(graphObjects);
    }

    void activate(void)
    {
        _topology.commit();
    }

    void deactivate(void)
    {
        _topology.disconnectAll();
        _topology.commit();
    }

    bool isActive(void)
    {
        
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
};
