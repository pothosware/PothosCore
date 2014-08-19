// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Proxy/Environment.hpp>
#include "GraphObjects/GraphObject.hpp"
#include <QObject>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <iostream>

struct ConnectionInfo
{
    std::string srcId, srcPort;
    std::string dstId, dstPort;
};

namespace Pothos
{
    class Topology;
}

class GraphBlock;

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
     * Get an eval environment object given a zone.
     */
    Pothos::Proxy getEvalEnvironment(const QString &zone);

    /*!
     * Get a proxy environment handle given a zone.
     */
    Pothos::ProxyEnvironment::Sptr getEnvironmentFromZone(const QString &zone);

    /*!
     * Get a thread pool handle given a zone.
     * The result is null for an invalid zone.
     */
    Pothos::Proxy getThreadPoolFromZone(const QString &zone);

    /*!
     * Evaluate the given graph block.
     * This call will set the block's error messages,
     * and determine the available connection ports.
     */
    Pothos::Proxy evalGraphBlock(GraphBlock *block);

    std::shared_ptr<Pothos::Topology> &getTopology(void)
    {
        return _topology;
    }

    void clear(void);

private:
    //! A host URI to a map of process names to server handles
    std::map<std::string, std::map<std::string, Pothos::Proxy>> _uriToProcessToServerHandle;

    std::map<QString, Pothos::Proxy> _zoneToThreadPool;

    std::map<QString, Pothos::Proxy> _zoneToEvalEnvironment;

    static std::vector<ConnectionInfo> getConnectionInfo(const GraphObjectList &graphObjects);

    //! The topology object thats executing this design
    std::shared_ptr<Pothos::Topology> _topology;

    //! a cache of block IDs to a pair(settings hash, block eval)
    std::map<QString, std::pair<std::string, Pothos::Proxy>> _idToBlockEval;

    void loadCachedStuffIntoBlock(GraphBlock *block);
    std::map<QString, std::pair<Poco::JSON::Array::Ptr, Poco::JSON::Array::Ptr>> _idToPortDesc;
    std::map<QString, std::map<QString, std::string>> _idToPropKeyToTypeStr;

    std::string _syslogListenPort;
};
