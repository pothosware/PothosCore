// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "TopologyEval.hpp"
#include "BlockEval.hpp"
#include <Pothos/Framework.hpp>

TopologyEval::TopologyEval(void):
    _topology(new Pothos::Topology())
{
    return;
}

TopologyEval::~TopologyEval(void)
{
    delete _topology;
}

void TopologyEval::acceptConnectionInfo(const ConnectionInfos &info)
{
    _newConnectionInfo = info;
}

void TopologyEval::acceptBlockEvals(const std::map<GraphBlock *, std::shared_ptr<BlockEval>> &info)
{
    _newBlockEvals = info;
}

void TopologyEval::update(void)
{
    //TODO actually have to check for block errors

    //all the connections in the new that are not in last
    ConnectionInfos newConnections;
    for (const auto &conn : _newConnectionInfo)
    {
        if (_lastConnectionInfo.count(conn) == 0)
        {
            newConnections.insert(conn);
        }
    }

    //all the connections in last that are not in new
    ConnectionInfos oldConnections;
    for (const auto &conn : _lastConnectionInfo)
    {
        if (_newConnectionInfo.count(conn) == 0)
        {
            oldConnections.insert(conn);
        }
    }

    //create new connections
    for (const auto &conn : newConnections)
    {
        const auto src = _newBlockEvals.at(conn.srcBlock)->getProxyBlock();
        const auto dst = _newBlockEvals.at(conn.dstBlock)->getProxyBlock();
        _topology->connect(src, conn.srcPort, dst, conn.dstPort);
    }

    //remove old connections
    for (const auto &conn : oldConnections)
    {
        const auto src = _lastBlockEvals.at(conn.srcBlock)->getProxyBlock();
        const auto dst = _lastBlockEvals.at(conn.dstBlock)->getProxyBlock();
        _topology->disconnect(src, conn.srcPort, dst, conn.dstPort);
    }

    //commit after changes
    _topology->commit();

    //stash data for the current state
    _lastBlockEvals = _newBlockEvals;
    _lastConnectionInfo = _newConnectionInfo;
}
