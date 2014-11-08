// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include "GraphObjects/GraphObject.hpp"
#include <string>
#include <vector>

/*!
 * Information about a connection between src and dst ports.
 * This is everything important we need to know about connections,
 * but extracted so we can access it in a thread-safe manner.
 */
struct ConnectionInfo
{
    //TODO use graph block ptr, not id...
    //the block ptr is already a unique key used to lookup the block evals
    std::string srcId, srcPort;
    std::string dstId, dstPort;
    std::string toString(void) const
    {
        return "("+srcId+"["+srcPort+"], "+dstId+"["+dstPort+"])";
    }
};

struct TopologyTraversal
{
    static std::vector<ConnectionInfo> getConnectionInfo(const GraphObjectList &graphObjects);
};
