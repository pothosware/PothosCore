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
#include <iostream>

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
    ~TopologyEngine(void)
    {
        this->clear();
    }

    //! Commit and run the topology
    void commitUpdate(const GraphObjectList &graphObjects);

    /*!
     * Get a proxy environment handle given a zone.
     */
    Pothos::ProxyEnvironment::Sptr getEnvironmentFromZone(const QString &zone);

    /*!
     * Get a thread pool handle given a zone.
     * The result is null for an invalid zone.
     */
    Pothos::Proxy getThreadPoolFromZone(const QString &zone);

    Pothos::Topology &getTopology(void)
    {
        return _topology;
    }

    void clear(void)
    {
        _topology.disconnectAll();
        _topology.commit();
    }

private:
    //! A host URI to a map of process names to server handles
    std::map<std::string, std::map<std::string, Pothos::Proxy>> _uriToProcessToServerHandle;

    std::map<QString, Pothos::Proxy> _zoneToThreadPool;

    static std::vector<ConnectionInfo> getConnectionInfo(const GraphObjectList &graphObjects);

    //! The topology object thats executing this design
    Pothos::Topology _topology;
};
