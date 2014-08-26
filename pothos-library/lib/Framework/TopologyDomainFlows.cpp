// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/TopologyImpl.hpp"

/***********************************************************************
 * helpers to deal with domain interaction
 **********************************************************************/
static bool isDomainCrossingAcceptable(
    const Port &mainPort,
    const std::vector<Port> &subPorts,
    const bool isInput
)
{
    auto mainDomain = mainPort.obj.callProxy(isInput?"input":"output", mainPort.name).call<std::string>("domain");

    bool allOthersAbdicate = true;
    std::set<std::string> subDomains;
    for (const auto &subPort : subPorts)
    {
        subDomains.insert(subPort.obj.callProxy(isInput?"output":"input", subPort.name).call<std::string>("domain"));
        auto actor = subPort.obj.callProxy("get:_actor");
        const auto subMode = actor.call<std::string>(isInput?"getOutputBufferMode":"getInputBufferMode", subPort.name, mainDomain);
        if (subMode != "ABDICATE") allOthersAbdicate = false;
    }

    //cant handle multiple domains
    if (subDomains.size() > 1) return false;

    assert(subDomains.size() == 1);
    const auto subDomain = *subDomains.begin();
    auto actor = mainPort.obj.callProxy("get:_actor");
    const auto mainMode = actor.call<std::string>(isInput?"getInputBufferMode":"getOutputBufferMode", mainPort.name, subDomain);

    //error always means we make a copy block
    if (mainMode == "ERROR") return false;

    //always good when we abdicate
    if (mainMode == "ABDICATE") return true;

    //otherwise the mode should be custom
    assert(mainMode == "CUSTOM");

    //cant handle custom with multiple upstream
    if (isInput and mainMode == "CUSTOM" and subPorts.size() > 1) return false;

    //if custom, the sub ports must abdicate
    if (mainMode == "CUSTOM" and not allOthersAbdicate) return false;

    return true;
}

static std::unordered_map<Port, Pothos::Proxy> domainInspection(
    const std::unordered_map<Port, std::vector<Port>> &ports,
    const bool isInput
)
{
    std::unordered_map<Port, Pothos::Proxy> bads;
    for (const auto &pair : ports)
    {
        if (isDomainCrossingAcceptable(pair.first, pair.second, isInput)) continue;
        auto registry = pair.first.obj.getEnvironment()->findProxy("Pothos/BlockRegistry");
        auto copier = registry.callProxy("/blocks/copier");
        copier.callVoid("setName", "DomainBridge");
        bads[pair.first] = copier;
    }

    return bads;
}

std::vector<Flow> Pothos::Topology::Impl::rectifyDomainFlows(const std::vector<Flow> &flatFlows)
{
    //map any given src or dst to its upstream/downstream ports
    std::unordered_map<Port, std::vector<Port>> srcs, dsts;
    for (const auto &flow : flatFlows)
    {
        for (const auto &subFlow : flatFlows)
        {
            if (subFlow.src == flow.src) srcs[flow.src].push_back(subFlow.dst);
            if (subFlow.dst == flow.dst) dsts[flow.dst].push_back(subFlow.src);
        }
    }

    //get a list of ports with domain problems
    auto badSrcsToCopier = domainInspection(srcs, false);
    auto badDstsToCopier = domainInspection(dsts, true);

    std::vector<Flow> domainSafeFlows;
    for (const auto &flow : flatFlows)
    {
        auto srcIt = badSrcsToCopier.find(flow.src);
        auto dstIt = badDstsToCopier.find(flow.dst);
        Pothos::Proxy copier;
        if (srcIt != badSrcsToCopier.end()) copier = srcIt->second;
        if (dstIt != badDstsToCopier.end()) copier = dstIt->second;
        if (copier)
        {
            //create the flows
            Flow srcFlow;
            srcFlow.src = flow.src;
            srcFlow.dst = makePort(copier, "0");

            Flow dstFlow;
            dstFlow.src = makePort(copier, "0");
            dstFlow.dst = flow.dst;

            //add the network flows to the overall list
            domainSafeFlows.push_back(srcFlow);
            domainSafeFlows.push_back(dstFlow);
        }
        else
        {
            domainSafeFlows.push_back(flow);
        }
    }

    return domainSafeFlows;
}
