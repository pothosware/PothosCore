// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Proxy/Environment.hpp>
#include <Pothos/Framework/Topology.hpp>
#include "GraphObjects/GraphObject.hpp"
#include <QObject>
#include <string>
#include <vector>
#include <map>

struct ConnectionInfo
{
    std::string srcId, srcPort;
    std::string dstId, dstPort;
};

class TopologyEngine : public QObject
{
    Q_OBJECT
public:
    TopologyEngine(QObject *parent);

    //! Commit and run the topology
    void commitUpdate(const GraphObjectList &graphObjects);

    /*!
     * Query the affinity zone from the cache
     */
    Pothos::ProxyEnvironment::Sptr getEnvironmentFromZone(const QString &zone);

private:
    //! A host URI to a map of process names to server handles
    std::map<std::string, std::map<std::string, Pothos::Proxy>> _uriToProcessToServer;

    static std::vector<ConnectionInfo> getConnectionInfo(const GraphObjectList &graphObjects);

    //! The topology object thats executing this design
    Pothos::Topology _topology;
};
