// Copyright (c) 2014-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/TopologyImpl.hpp"
#include <future>
#include <iostream>
#include <algorithm>
#include <map>

/***********************************************************************
 * helpers to call into remote worker
 **********************************************************************/
static std::string getBufferMode(const Port &port, const std::string &domain, const bool &isInput)
{
    auto actor = port.obj.get("_actor");
    return actor.call<std::string>("getBufferMode", port.name, domain, isInput);
}

static std::string getDomain(const Port &port, const bool &isInput)
{
    return port.obj.call(isInput?"input":"output", port.name).call<std::string>("domain");
}

/***********************************************************************
 * helpers to deal with domain interaction
 **********************************************************************/

/*!
 * Is this domain crossing possible between mainPort and all connected subPorts?
 */
static bool isDomainCrossingAcceptable(
    const Port &mainPort,
    const std::vector<Port> &subPorts,
    const bool isInput
)
{
    auto mainDomain = getDomain(mainPort, isInput);

    bool allOthersAbdicate = true;
    std::set<std::string> subDomains;
    for (const auto &subPort : subPorts)
    {
        subDomains.insert(getDomain(subPort, not isInput));
        const auto subMode = getBufferMode(subPort, mainDomain, not isInput);
        if (subMode != "ABDICATE") allOthersAbdicate = false;
    }

    //cant handle multiple domains
    if (subDomains.size() > 1) return false;

    assert(subDomains.size() == 1);
    const auto subDomain = *subDomains.begin();
    const auto mainMode = getBufferMode(mainPort, subDomain, isInput);

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

/*!
 * Get a copier block for a domain crossing between mainPort and all connected subPorts.
 * If the copier block is not needed to handle this domain crossing, return a null Proxy.
 */
static Pothos::Proxy getCopierForDomainCrossing(
    const Port &mainPort,
    const std::vector<Port> &subPorts,
    const bool isInput
)
{
    if (isDomainCrossingAcceptable(mainPort, subPorts, isInput)) return Pothos::Proxy();
    auto registry = mainPort.obj.getEnvironment()->findProxy("Pothos/BlockRegistry");
    auto copier = registry.call("/blocks/copier");
    copier.call("setName", "DomainBridge");
    return copier;
}

/*!
 * Get a future for each port to inspect it for domain crossing.
 */
static std::unordered_map<Port, std::shared_future<Pothos::Proxy>> domainInspection(
    const std::unordered_map<Port, std::vector<Port>> &ports,
    const bool isInput
)
{
    std::unordered_map<Port, std::shared_future<Pothos::Proxy>> copiers;
    for (const auto &pair : ports)
    {
        copiers[pair.first] = std::async(std::launch::async,
            &getCopierForDomainCrossing, pair.first, pair.second, isInput);
    }
    return copiers;
}

/***********************************************************************
 * domain crossing implementation -- insert copy blocks where needed
 **********************************************************************/
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
        auto srcCopier = badSrcsToCopier.at(flow.src).get();
        auto dstCopier = badDstsToCopier.at(flow.dst).get();
        Pothos::Proxy copier;
        if (srcCopier) copier = srcCopier;
        if (dstCopier) copier = dstCopier;
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
            if (std::find(domainSafeFlows.begin(), domainSafeFlows.end(), srcFlow) == domainSafeFlows.end()) domainSafeFlows.push_back(srcFlow);
            if (std::find(domainSafeFlows.begin(), domainSafeFlows.end(), dstFlow) == domainSafeFlows.end()) domainSafeFlows.push_back(dstFlow);
        }
        else
        {
            domainSafeFlows.push_back(flow);
        }
    }

    return domainSafeFlows;
}
