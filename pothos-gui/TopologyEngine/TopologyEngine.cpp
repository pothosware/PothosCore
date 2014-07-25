// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGuiUtils.hpp" //get object map
#include "TopologyEngine/TopologyEngine.hpp"
#include "GraphObjects/GraphBlock.hpp"
#include "AffinitySupport/AffinityZonesDock.hpp"
#include <Pothos/Proxy.hpp>
#include <Pothos/Remote.hpp>
#include <Pothos/Framework.hpp>
#include <Poco/URI.h>
#include <Poco/NumberParser.h>
#include <cassert>

TopologyEngine::TopologyEngine(QObject *parent):
    QObject(parent)
{
    return;
}

void TopologyEngine::commitUpdate(const GraphObjectList &graphObjects)
{
    //get all block proxy objects
    std::map<std::string, Pothos::Proxy> idToBlock;
    for (auto obj : graphObjects)
    {
        auto block = dynamic_cast<GraphBlock *>(obj);
        if (block == nullptr) continue;
        
        //NOPE! FIXME TODO
        
        //idToBlock[block->getId().toStdString()] = block->getBlockEval().callProxy("getProxyBlock");
    }

    //make all of the connections
    _topology.disconnectAll();
    for (const auto &conn : getConnectionInfo(graphObjects))
    {
        _topology.connect(
            idToBlock.at(conn.srcId), conn.srcPort,
            idToBlock.at(conn.dstId), conn.dstPort);
    }

    //commit the new design
    _topology.commit();
}

Pothos::ProxyEnvironment::Sptr TopologyEngine::getEnvironmentFromZone(const QString &zone)
{
    auto dock = dynamic_cast<AffinityZonesDock *>(getObjectMap()["affinityZonesDock"]);
    assert(dock != nullptr);
    auto config = dock->zoneToConfig(zone);
    auto hostUri = config->getValue<std::string>("hostUri");
    auto processName = config->getValue<std::string>("processName");

    //find if the environment already exists
    if (_uriToProcessToServer.find(hostUri) == _uriToProcessToServer.end())
    {
        auto processToServer = _uriToProcessToServer.at(hostUri);
        if (processToServer.find(processName) == processToServer.end())
        {
            //talk to the server and spawn a new process with a new server
            auto env = Pothos::RemoteClient(hostUri).makeEnvironment("managed");
            auto serverHandle = env->findProxy("Pothos/RemoteServer").callProxy("new", "tcp://0.0.0.0");
            _uriToProcessToServer[hostUri][processName] = serverHandle;
        }
    }

    //connect to the new server and make the communication environment
    auto serverHandle = _uriToProcessToServer.at(hostUri).at(processName);
    auto actualPort = serverHandle.call<std::string>("getActualPort");
    Poco::URI newHostUri(hostUri);
    newHostUri.setPort(Poco::NumberParser::parseUnsigned(actualPort));
    return Pothos::RemoteClient(newHostUri.toString()).makeEnvironment("managed");
}
