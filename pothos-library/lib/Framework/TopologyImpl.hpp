// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Framework/Topology.hpp>
#include "Framework/PortsAndFlows.hpp"
#include <unordered_map>
#include <map>
#include <vector>
#include <string>

/***********************************************************************
 * implementation guts
 **********************************************************************/
struct Pothos::Topology::Impl
{
    ThreadPool threadPool;
    std::vector<Flow> flows;
    std::vector<Flow> activeFlatFlows;
    std::unordered_map<Flow, std::pair<Flow, Flow>> flowToNetgressCache;
    std::vector<Flow> createNetworkFlows(const std::vector<Flow> &);
    std::vector<Flow> rectifyDomainFlows(const std::vector<Flow> &);
    std::vector<std::string> inputPortNames;
    std::vector<std::string> outputPortNames;
    std::map<std::string, PortInfo> inputPortInfo;
    std::map<std::string, PortInfo> outputPortInfo;
    std::map<std::string, Callable> calls;
};

inline std::vector<Pothos::Proxy> getObjSetFromFlowList(const std::vector<Flow> &flows, const std::vector<Flow> &excludes = std::vector<Flow>())
{
    std::map<std::string, Pothos::Proxy> uniques;
    for (const auto &flow : flows)
    {
        uniques[flow.src.obj.call<std::string>("uid")] = flow.src.obj;
        uniques[flow.dst.obj.call<std::string>("uid")] = flow.dst.obj;
    }
    for (const auto &flow : excludes)
    {
        uniques.erase(flow.src.obj.call<std::string>("uid"));
        uniques.erase(flow.dst.obj.call<std::string>("uid"));
    }
    std::vector<Pothos::Proxy> set;
    for (const auto &pair : uniques) set.push_back(pair.second);
    return set;
}
