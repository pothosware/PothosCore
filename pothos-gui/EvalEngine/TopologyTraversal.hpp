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
