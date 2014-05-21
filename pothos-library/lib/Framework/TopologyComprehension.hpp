// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Framework/Topology.hpp>
#include <unordered_map>
#include <vector>
#include <functional> //std::hash

std::string getUid(const Pothos::Object &o);

/***********************************************************************
 * A port contains a worker object and a port name
 **********************************************************************/
struct Port
{
    Pothos::Object obj;
    std::string name;
};

bool operator==(const Port &lhs, const Port &rhs)
{
    if (getUid(lhs.obj) != getUid(rhs.obj)) return false;
    if (lhs.name != rhs.name) return false;
    return true;
}

namespace std
{
    template<>
    struct hash<Port>
    {
        typedef Port argument_type;
        typedef std::size_t value_type;

        value_type operator()(argument_type const& s) const
        {
            return std::hash<std::string>()(getUid(s.obj)) ^
            (std::hash<std::string>()(s.name) << 1);
        }
    };
}

/***********************************************************************
 * A flow contains all the arguments for a connection
 **********************************************************************/
struct Flow
{
    Port src;
    Port dst;
};

bool operator==(const Flow &lhs, const Flow &rhs)
{
    if (not (rhs.src == lhs.src)) return false;
    if (not (rhs.dst == lhs.dst)) return false;
    return true;
}

namespace std
{
    template<>
    struct hash<Flow>
    {
        typedef Flow argument_type;
        typedef std::size_t value_type;

        value_type operator()(argument_type const& s) const
        {
            return std::hash<Port>()(s.dst) ^
            (std::hash<Port>()(s.dst) << 1);
        }
    };
}

/***********************************************************************
 * implementation guts
 **********************************************************************/
struct Pothos::Topology::Impl
{
    std::vector<Flow> flows;
    std::vector<Flow> activeFlatFlows;
    std::unordered_map<Flow, std::pair<Flow, Flow>> flowToNetgressCache;
    std::vector<Flow> createNetworkFlows(void);
};
