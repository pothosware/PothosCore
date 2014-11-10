// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include "GraphObjects/GraphObject.hpp"
#include <Pothos/Proxy/Proxy.hpp>
#include <QObject>
#include <string>
#include <vector>
#include <memory>
#include <map>

class GraphObject;
class GraphBlock;
class BlockEval;

namespace Pothos
{
    class Topology;
}

/*!
 * Information about a connection between src and dst ports.
 * This is everything important we need to know about connections,
 * but extracted so we can access it in a thread-safe manner.
 */
struct ConnectionInfo
{
    ConnectionInfo(void):
        srcBlock(nullptr),
        dstBlock(nullptr){}
    GraphBlock *srcBlock, *dstBlock;
    std::string srcPort, dstPort;
};

//! typedef for multiple connection informations
typedef std::vector<ConnectionInfo> ConnectionInfos;

/*!
 * TopologyEval takes up to date connection information
 * and creates topology connections between block objects.
 */
class TopologyEval : public QObject
{
    Q_OBJECT
public:

    TopologyEval(void);

    ~TopologyEval(void);

    //! helper to parse graph objects into a list of thread-safe info
    static ConnectionInfos getConnectionInfo(const GraphObjectList &graphObjects);

    /*!
     * Pass-in up-to-date connection information.
     */
    void acceptConnectionInfo(const ConnectionInfos &);

    /*!
     * Pass-in up-to-date block eval objects.
     */
    void acceptBlockEvals(const std::map<GraphBlock *, std::shared_ptr<BlockEval>> &);

    /*!
     * Perform update work after changes applied.
     */
    void update(void);

    //! Get access to the active topology
    Pothos::Topology *getTopology(void) const
    {
        return _topology;
    }

    //! An error caused the topology to go into failure state
    bool isFailureState(void) const
    {
        return _failureState;
    }

private:
    ConnectionInfos _newConnectionInfo;
    ConnectionInfos _lastConnectionInfo;

    std::map<GraphBlock *, std::shared_ptr<BlockEval>> _newBlockEvals;
    std::map<GraphBlock *, std::shared_ptr<BlockEval>> _lastBlockEvals;

    //! The topology object thats executing this design
    Pothos::Topology *_topology;

    bool _failureState;
};
