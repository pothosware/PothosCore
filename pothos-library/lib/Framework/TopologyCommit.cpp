// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/TopologyImpl.hpp"
#include <Pothos/Framework/Block.hpp>
#include <Pothos/Framework/Exception.hpp>
#include <Poco/Format.h>
#include <iostream>
#include <future>

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
    std::vector<std::pair<std::string, Pothos::Proxy>> infoReceivers;

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
            assert(dsts.size() == 1); //this must be true if the previous logic was good
            manager = dst.obj.callProxy("get:_actor").callProxy("getBufferManager", dst.name, srcDomain, true);
        }

        //otherwise create a generic manager and install it to the source
        else
        {
            assert(srcMode == "ABDICATE"); //this must be true if the previous logic was good
            assert(dstMode == "ABDICATE");
            manager = src.obj.callProxy("get:_actor").callProxy("getBufferManager", src.name, dstDomain, false);
        }

        auto result = src.obj.callProxy("get:_actor").callProxy("setOutputBufferManager", src.name, manager);
        const auto msg = Poco::format("%s.setOutputBufferManager(%s)", src.obj.call<std::string>("getName"), src.name);
        infoReceivers.push_back(std::make_pair(msg, result));
    }

    //check all subscribe message results
    std::string errors;
    for (auto infoReceiver : infoReceivers)
    {
        const auto &msg = infoReceiver.second.call<std::string>("WaitInfo");
        if (not msg.empty()) errors.append(infoReceiver.first+": "+msg+"\n");
    }
    if (not errors.empty()) Pothos::TopologyConnectError("Pothos::Exectutor::commit()", errors);
}

/***********************************************************************
 * Helpers to implement port subscription
 **********************************************************************/
static void updateFlows(const std::vector<Flow> &flows, const std::string &action)
{
    const bool isInputAction = action.find("INPUT") != std::string::npos;

    //result list is used to ack all subscribe messages
    std::vector<std::pair<std::string, Pothos::Proxy>> infoReceivers;

    //add new data acceptors
    for (const auto &flow : flows)
    {
        const auto &pri = isInputAction?flow.src:flow.dst;
        const auto &sec = isInputAction?flow.dst:flow.src;

        auto actor = pri.obj.callProxy("get:_actor");
        auto result = actor.callProxy("sendPortSubscriberMessage", action, pri.name, sec.obj.callProxy("getPointer"), sec.name);
        const auto msg = Poco::format("%s.sendPortSubscriberMessage(%s)", pri.obj.call<std::string>("getName"), action);
        infoReceivers.push_back(std::make_pair(msg, result));
    }

    //check all subscribe message results
    std::string errors;
    for (auto infoReceiver : infoReceivers)
    {
        const auto &msg = infoReceiver.second.call<std::string>("WaitInfo");
        if (not msg.empty()) errors.append(infoReceiver.first+": "+msg+"\n");
    }
    if (not errors.empty()) Pothos::TopologyConnectError("Pothos::Exectutor::commit()", errors);
}

/***********************************************************************
 * Topology commit
 **********************************************************************/

static void doCommit(const Pothos::Proxy &proxy)
{
    proxy.callVoid("commit");
}

void Pothos::Topology::commit(void)
{
    //1) flatten the topology
    auto flatFlows = _impl->squashFlows(_impl->flows);

    //2) create network iogress blocks when needed
    flatFlows = _impl->createNetworkFlows(flatFlows);
//*
    bool hasRemoteTopologies = not _impl->flowToNetgressCache.empty();

    //create remote topologies for all environments
    for (const auto &obj : getObjSetFromFlowList(flatFlows))
    {
        auto upid = obj.getEnvironment()->getUniquePid();
        if (upid != Pothos::ProxyEnvironment::getLocalUniquePid()) hasRemoteTopologies = true;
        if (_impl->remoteTopologies.count(upid) != 0) continue;
        _impl->remoteTopologies[upid] = obj.getEnvironment()->findProxy("Pothos/Topology").callProxy("make");
    }

    if (hasRemoteTopologies)
    {
        //clear connections on old topologies
        for (const auto &pair : _impl->remoteTopologies)
        {
            pair.second.callVoid("disconnectAll");
        }

        //load each topology with connections from flat flows
        for (const auto &flow : flatFlows)
        {
            auto upid = flow.src.obj.getEnvironment()->getUniquePid();
            assert(upid == flow.dst.obj.getEnvironment()->getUniquePid());
            _impl->remoteTopologies[upid].callVoid("connect", flow.src.obj, flow.src.name, flow.dst.obj, flow.dst.name);
        }

        //call commit on all topologies
        std::vector<std::future<void>> futures;
        for (const auto &pair : _impl->remoteTopologies)
        {
            futures.push_back(std::async(std::launch::async, &doCommit, pair.second));
        }
        for (auto &future : futures) future.wait();
/*
        for (const auto &pair : _impl->remoteTopologies)
        {
            pair.second.callVoid("commit");
        }
        */

        _impl->activeFlatFlows = flatFlows;
        return;
    }
    //*/

    //3) deal with domain crossing
    flatFlows = _impl->rectifyDomainFlows(flatFlows);

    const auto &activeFlatFlows = _impl->activeFlatFlows;

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

    //set thread pools for all blocks in this process
    if (this->getThreadPool()) for (auto block : getObjSetFromFlowList(flatFlows))
    {
        if (block.getEnvironment()->getUniquePid() != Pothos::ProxyEnvironment::getLocalUniquePid()) continue; //is the block local?
        block.call<Block *>("getPointer")->setThreadPool(this->getThreadPool());
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
    std::vector<std::pair<std::string, Pothos::Proxy>> infoReceivers;

    //send activate to all new blocks not already in active flows
    for (auto block : getObjSetFromFlowList(newFlows, activeFlatFlows))
    {
        std::cout << "sendActivateMessage " << block.call<std::string>("getName") << block.call<std::string>("uid") << std::endl;
        auto actor = block.callProxy("get:_actor");
        const auto msg = Poco::format("%s.sendActivateMessage()", block.call<std::string>("getName"));
        infoReceivers.push_back(std::make_pair(msg, actor.callProxy("sendActivateMessage")));
    }

    //update current flows
    _impl->activeFlatFlows = flatFlows;

    //send deactivate to all old blocks not in current active flows
    for (auto block : getObjSetFromFlowList(oldFlows, _impl->activeFlatFlows))
    {
        std::cout << "sendDeactivateMessage " << block.call<std::string>("getName") << block.call<std::string>("uid") << std::endl;
        auto actor = block.callProxy("get:_actor");
        const auto msg = Poco::format("%s.sendDeactivateMessage()", block.call<std::string>("getName"));
        infoReceivers.push_back(std::make_pair(msg, actor.callProxy("sendDeactivateMessage")));
    }

    //check all de/activate message results
    std::string errors;
    for (auto infoReceiver : infoReceivers)
    {
        const auto &msg = infoReceiver.second.call<std::string>("WaitInfo");
        if (not msg.empty()) errors.append(infoReceiver.first+": "+msg+"\n");
    }
    if (not errors.empty()) Pothos::TopologyConnectError("Pothos::Exectutor::commit()", errors);

    //remove disconnections from the cache if present
    for (auto flow : oldFlows)
    {
        for (auto it = _impl->flowToNetgressCache.begin(); it != _impl->flowToNetgressCache.end(); it++)
        {
            if (flow == it->second.first or flow == it->second.second)
            {
                _impl->flowToNetgressCache.erase(it);
                break;
            }
        }
    }
}
