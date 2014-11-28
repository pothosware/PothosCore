// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/TopologyImpl.hpp"
#include "Framework/WorkerActor.hpp"
#include <Pothos/Framework/Block.hpp>
#include <Pothos/Framework/Exception.hpp>
#include <Poco/Format.h>
#include <iostream>
#include <future>

typedef std::shared_ptr<InfoReceiver<std::string>> StrInfoReceiver;

struct FutureInfo
{
    FutureInfo(const std::string &what, const Pothos::Proxy &block, const StrInfoReceiver &result):
        what(what), block(block), result(result){}
    std::string what;
    Pothos::Proxy block;
    StrInfoReceiver result;
};

std::string collectFutureInfoErrors(const std::vector<FutureInfo> infoFutures)
{
    std::string errors;
    for (auto future : infoFutures)
    {
        const auto &msg = future.result->WaitInfo();
        if (not msg.empty()) errors.append(future.block.call<std::string>("getName")+"."+future.what+": "+msg+"\n");
    }
    return errors;
}

/***********************************************************************
 * helpers to deal with buffer managers
 **********************************************************************/
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
                    throw Pothos::Exception("Pothos::Topology::installBufferManagers",
                        "rectifyDomainFlows() logic does not /yet/ handle multiple destinations w/ custom buffer managers");
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

        auto result = src.obj.callProxy("get:_actor").call<StrInfoReceiver>("setOutputBufferManager", src.name, manager);
        infoFutures.push_back(FutureInfo(Poco::format("setOutputBufferManager(%s)", src.name), src.obj, result));
    }

    //check all subscribe message results
    const auto errors = collectFutureInfoErrors(infoFutures);
    if (not errors.empty()) throw Pothos::TopologyConnectError(errors);
}

/***********************************************************************
 * Helpers to implement port subscription
 **********************************************************************/
static void updateFlows(const std::vector<Flow> &flows, const std::string &action)
{
    const bool isInputAction = action.find("INPUT") != std::string::npos;

    //result list is used to ack all subscribe messages
    std::vector<FutureInfo> infoFutures;

    //add new data acceptors
    for (const auto &flow : flows)
    {
        const auto &pri = isInputAction?flow.src:flow.dst;
        const auto &sec = isInputAction?flow.dst:flow.src;

        auto actor = pri.obj.callProxy("get:_actor");
        auto result = actor.call<StrInfoReceiver>("sendPortSubscriberMessage", action, pri.name, sec.obj.callProxy("getPointer"), sec.name);
        infoFutures.push_back(FutureInfo(Poco::format("sendPortSubscriberMessage(%s)", action), pri.obj, result));
    }

    //check all subscribe message results
    const auto errors = collectFutureInfoErrors(infoFutures);
    if (not errors.empty()) throw Pothos::TopologyConnectError(errors);
}

/***********************************************************************
 * Sub Topology commit on flattened flows
 **********************************************************************/
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
    updateFlows(newFlows, "SUBINPUT");

    //add new data providers
    updateFlows(newFlows, "SUBOUTPUT");

    //remove old data providers
    updateFlows(oldFlows, "UNSUBOUTPUT");

    //remove old data acceptors
    updateFlows(oldFlows, "UNSUBINPUT");

    //install buffer managers on sources for all new flows
    //Sometimes this will replace previous buffer managers.
    installBufferManagers(newFlows);

    //result list is used to ack all de/activate messages
    std::vector<FutureInfo> infoFutures;

    //send activate to all new blocks not already in active flows
    for (auto block : getObjSetFromFlowList(newFlows, activeFlatFlows))
    {
        auto result = block.callProxy("get:_actor").call<StrInfoReceiver>("sendActivateMessage");
        infoFutures.push_back(FutureInfo("sendActivateMessage()", block, result));
    }

    //update current flows
    _impl->activeFlatFlows = flatFlows;

    //send deactivate to all old blocks not in current active flows
    for (auto block : getObjSetFromFlowList(oldFlows, _impl->activeFlatFlows))
    {
        auto result = block.callProxy("get:_actor").call<StrInfoReceiver>("sendDeactivateMessage");
        infoFutures.push_back(FutureInfo("sendDeactivateMessage()", block, result));
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
    //1) flatten the topology
    auto squashedFlows = _impl->squashFlows(_impl->flows);

    //2) create network iogress blocks when needed
    auto flatFlows = _impl->createNetworkFlows(squashedFlows);

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
    for (const auto &flow : squashedFlows)
    {
        const auto port = envTagPort(flow.src, flow.dst);
        auto it = _impl->srcToNetgressCache.find(port);
        if (it == _impl->srcToNetgressCache.end()) continue;
        newNetgressCache[it->first] = it->second;
    }
    _impl->srcToNetgressCache = newNetgressCache;
}
