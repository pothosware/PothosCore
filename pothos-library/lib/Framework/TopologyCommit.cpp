// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/TopologyImpl.hpp"
#include <Pothos/Framework/Block.hpp>
#include <Pothos/Framework/Exception.hpp>
#include <Poco/Format.h>
#include <iostream>
#include <future>

struct FutureInfo
{
    FutureInfo(const std::string &what, const Pothos::Proxy &block, const std::shared_future<void> &result):
        what(what), block(block), result(result){}
    std::string what;
    Pothos::Proxy block;
    std::shared_future<void> result;
};

std::string collectFutureInfoErrors(const std::vector<FutureInfo> infoFutures)
{
    std::string errors;
    for (auto future : infoFutures)
    {
        POTHOS_EXCEPTION_TRY
        {
            future.result.get();
        }
        POTHOS_EXCEPTION_CATCH (const Pothos::Exception &ex)
        {
            errors.append(future.block.call<std::string>("getName")+"."+future.what+": "+ex.message()+"\n");
        }
    }
    return errors;
}

/***********************************************************************
 * helpers to deal with buffer managers
 **********************************************************************/
static void setOutputBufferManager(const Port &src, const Pothos::Proxy &manager)
{
    src.obj.callProxy("get:_actor").callVoid("setOutputBufferManager", src.name, manager);
}

static void installBufferManagers(const std::vector<Flow> &flatFlows)
{
    //map of a source port to all destination ports
    std::unordered_map<Port, std::vector<Port>> srcs;
    for (const auto &flow : flatFlows)
    {
        for (const auto &subFlow : flatFlows)
        {
            if (subFlow.src == flow.src) srcs[flow.src].push_back(subFlow.dst);
        }
    }

    //result list is used to ack all install messages
    std::vector<FutureInfo> infoFutures;

    //for each source port -- install managers
    for (const auto &pair : srcs)
    {
        auto src = pair.first;
        auto dsts = pair.second;
        auto dst = dsts.at(0);
        Pothos::Proxy manager;

        auto srcDomain = src.obj.callProxy("output", src.name).call<std::string>("domain");
        auto dstDomain = dst.obj.callProxy("input", dst.name).call<std::string>("domain");

        auto srcMode = src.obj.callProxy("get:_actor").call<std::string>("getOutputBufferMode", src.name, dstDomain);
        auto dstMode = dst.obj.callProxy("get:_actor").call<std::string>("getInputBufferMode", dst.name, srcDomain);

        //check if the source provides a manager and install it to the source
        if (srcMode == "CUSTOM")
        {
            manager = src.obj.callProxy("get:_actor").callProxy("getBufferManager", src.name, dstDomain, false);
        }

        //check if the destination provides a manager and install it to the source
        else if (dstMode == "CUSTOM")
        {
            for (const auto &otherDst : dsts)
            {
                if (otherDst == dst) continue;
                if (otherDst.obj.callProxy("get:_actor").call<std::string>("getInputBufferMode", dst.name, srcDomain) != "ABDICATE")
                {
                    throw Pothos::Exception("Pothos::Topology::installBufferManagers", Poco::format("%s->%s\n"
                        "rectifyDomainFlows() logic does not /yet/ handle multiple destinations w/ custom buffer managers",
                        src.toString(), otherDst.toString()));
                }
            }
            manager = dst.obj.callProxy("get:_actor").callProxy("getBufferManager", dst.name, srcDomain, true);
        }

        //otherwise create a generic manager and install it to the source
        else
        {
            assert(srcMode == "ABDICATE"); //this must be true if the previous logic was good
            assert(dstMode == "ABDICATE");
            manager = src.obj.callProxy("get:_actor").callProxy("getBufferManager", src.name, dstDomain, false);
        }

        std::shared_future<void> result(std::async(std::launch::async, setOutputBufferManager, src, manager));
        infoFutures.push_back(FutureInfo(Poco::format("setOutputBufferManager(%s)", src.name), src.obj, result));
    }

    //check all subscribe message results
    const auto errors = collectFutureInfoErrors(infoFutures);
    if (not errors.empty()) throw Pothos::TopologyConnectError(errors);
}

/***********************************************************************
 * Helpers to implement port subscription
 **********************************************************************/
static void subscribePort(const Port &src, const Port &dst, const std::string &action)
{
    {
        auto actor = src.obj.callProxy("get:_actor");
        actor.callVoid("subscribeInput", action, src.name, dst.obj.callProxy("input", dst.name));
    }
    {
        auto actor = dst.obj.callProxy("get:_actor");
        actor.callVoid("subscribeOutput", action, dst.name, src.obj.callProxy("output", src.name));
    }
}

static void updateFlows(const std::vector<Flow> &flows, const std::string &action)
{
    //result list is used to ack all subscribe messages
    std::vector<FutureInfo> infoFutures;

    //add new data acceptors
    for (const auto &flow : flows)
    {
        std::shared_future<void> result(std::async(std::launch::async, subscribePort, flow.src, flow.dst, action));
        infoFutures.push_back(FutureInfo(action, flow.src.obj, result));
    }

    //check all subscribe message results
    const auto errors = collectFutureInfoErrors(infoFutures);
    if (not errors.empty()) throw Pothos::TopologyConnectError(errors);
}

/***********************************************************************
 * complete pass-through flows
 **********************************************************************/
static std::vector<Flow> completePassThroughFlows(const std::vector<Flow> &flows)
{
    std::vector<Flow> outFlows;
    //std::cout << "completePassThroughFlows:" << std::endl;
    //for (const auto &flow : flows) std::cout << "  " << flow.toString() << std::endl;

    //try to complete pass-through flows and add it to the out flow list
    for (auto &flow : flows)
    {
        if (flow.src.obj or flow.dst.obj) continue;
        for (auto &flowTail : flows)
        {
            if (not flowTail.dst.obj) continue;
            for (auto &flowHead : flows)
            {
                if (not flowHead.src.obj) continue;
                if (flow.src == flowTail.src and flow.dst == flowHead.dst)
                {
                    //create the new completed flow
                    Flow newFlow;
                    newFlow.src = flowHead.src;
                    newFlow.dst = flowTail.dst;
                    outFlows.push_back(newFlow);
                    //std::cout << "NEW " << newFlow.toString() << std::endl;
                }
            }
        }
    }

    //add all already completed flows to the output flow
    for (auto &flow : flows)
    {
        if (flow.src.obj and flow.dst.obj)  outFlows.push_back(flow);
    }

    return outFlows;
}

/***********************************************************************
 * Sub Topology commit on flattened flows
 **********************************************************************/
static void setActiveState(const Pothos::Proxy &block, const bool state)
{
    block.callProxy("get:_actor").callVoid(state?"setActiveStateOn":"setActiveStateOff");
}

void topologySubCommit(Pothos::Topology &topology)
{
    auto &_impl = topology._impl;
    const auto &activeFlatFlows = _impl->activeFlatFlows;
    const auto &flatFlows = _impl->flows;

    //new flows are in flat flows but not in current
    std::vector<Flow> newFlows;
    for (const auto &flow : flatFlows)
    {
        if (std::find(activeFlatFlows.begin(), activeFlatFlows.end(), flow) == activeFlatFlows.end()) newFlows.push_back(flow);
    }

    //old flows are in current and not in flat flows
    std::vector<Flow> oldFlows;
    for (const auto &flow : _impl->activeFlatFlows)
    {
        if (std::find(flatFlows.begin(), flatFlows.end(), flow) == flatFlows.end()) oldFlows.push_back(flow);
    }

    //add new data acceptors
    updateFlows(newFlows, "add");

    //remove old data acceptors
    updateFlows(oldFlows, "remove");

    //install buffer managers on sources for all new flows
    //Sometimes this will replace previous buffer managers.
    installBufferManagers(newFlows);

    //result list is used to ack all de/activate messages
    std::vector<FutureInfo> infoFutures;

    //send activate to all new blocks not already in active flows
    for (auto block : getObjSetFromFlowList(newFlows, activeFlatFlows))
    {
        std::shared_future<void> result(std::async(std::launch::async, setActiveState, block, true));
        infoFutures.push_back(FutureInfo("activate()", block, result));
    }

    //update current flows
    _impl->activeFlatFlows = flatFlows;

    //send deactivate to all old blocks not in current active flows
    for (auto block : getObjSetFromFlowList(oldFlows, _impl->activeFlatFlows))
    {
        std::shared_future<void> result(std::async(std::launch::async, setActiveState, block, false));
        infoFutures.push_back(FutureInfo("deactivate()", block, result));
    }

    //check all de/activate message results
    const auto errors = collectFutureInfoErrors(infoFutures);
    if (not errors.empty()) throw Pothos::TopologyConnectError(errors);
}

/***********************************************************************
 * Topology commit
 **********************************************************************/
static void subCommitFutureTask(const Pothos::Proxy &proxy)
{
    proxy.callVoid("subCommit");
}

void Pothos::Topology::commit(void)
{
    //0) flatten the topology
    auto squashedFlows = _impl->squashFlows(_impl->flows);

    //1) complete the pass-through flows
    auto completeFlows = completePassThroughFlows(squashedFlows);

    //2) create network iogress blocks when needed
    auto flatFlows = _impl->createNetworkFlows(completeFlows);

    //3) deal with domain crossing
    flatFlows = _impl->rectifyDomainFlows(flatFlows);

    //create remote topologies for all environments
    for (const auto &obj : getObjSetFromFlowList(flatFlows))
    {
        auto upid = obj.getEnvironment()->getUniquePid();
        if (_impl->remoteTopologies.count(upid) != 0) continue;
        _impl->remoteTopologies[upid] = obj.getEnvironment()->findProxy("Pothos/Topology").callProxy("make");
    }

    //clear connections on old topologies
    for (const auto &pair : _impl->remoteTopologies) pair.second.callVoid("disconnectAll");

    //load each topology with connections from flat flows
    for (const auto &flow : flatFlows)
    {
        auto upid = flow.src.obj.getEnvironment()->getUniquePid();
        assert(upid == flow.dst.obj.getEnvironment()->getUniquePid());
        _impl->remoteTopologies[upid].callVoid("connect", flow.src.obj, flow.src.name, flow.dst.obj, flow.dst.name);
    }

    //Call commit on all sub-topologies:
    //Use futures so all sub-topologies commit at the same time,
    //which is important for network source/sink pairs to connect.
    std::vector<std::future<void>> futures;
    for (const auto &pair : _impl->remoteTopologies)
    {
        futures.push_back(std::async(std::launch::async, &subCommitFutureTask, pair.second));
    }

    //wait on futures and collect errors
    std::string errors;
    for (auto &future : futures)
    {
        try {future.get();}
        catch (const Exception &ex)
        {
            errors.append(ex.message()+"\n");
        }
    }
    if (not errors.empty()) throw Pothos::TopologyConnectError("Pothos::Topology::commit()", errors);

    //set thread pools for all blocks in this process
    if (this->getThreadPool()) for (auto block : getObjSetFromFlowList(flatFlows))
    {
        if (block.getEnvironment()->getUniquePid() != Pothos::ProxyEnvironment::getLocalUniquePid()) continue; //is the block local?
        block.call<Block *>("getPointer")->setThreadPool(this->getThreadPool());
    }

    _impl->activeFlatFlows = flatFlows;

    //Remove disconnections from the cache if present
    //by only saving in the curretly in-use flows.
    std::unordered_map<Port, std::pair<Pothos::Proxy, Pothos::Proxy>> newNetgressCache;
    for (const auto &flow : completeFlows)
    {
        const auto port = envTagPort(flow.src, flow.dst);
        auto it = _impl->srcToNetgressCache.find(port);
        if (it == _impl->srcToNetgressCache.end()) continue;
        newNetgressCache[it->first] = it->second;
    }
    _impl->srcToNetgressCache = newNetgressCache;
}
