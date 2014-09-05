// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/TopologyImpl.hpp"
#include <Pothos/System/HostInfo.hpp>
#include <Pothos/Remote.hpp>
#include <Poco/Format.h>
#include <future>

/***********************************************************************
 * helpers to create network iogress flows
 **********************************************************************/
std::pair<Flow, Flow> Pothos::Topology::Impl::createNetworkFlow(const Flow &flow)
{
    //default behaviour: the sink binds, the source connects
    auto bindEnv = flow.src.obj.getEnvironment();
    auto connEnv = flow.dst.obj.getEnvironment();
    Pothos::Proxy netConn, netBind;
    auto netSink = std::ref(netBind);
    auto netSource = std::ref(netConn);
    auto netBindPath = "/blocks/network_sink";
    auto netConnPath = "/blocks/network_source";

    //If the bind environment is local, swap them to bind on the remote host.
    //The reason for this swap, is that we dont know the local's external IP
    //to which the remote host can connect to, so we just do the reverse.
    if (Pothos::System::HostInfo::get().nodeId == bindEnv->getNodeId())
    {
        std::swap(bindEnv, connEnv);
        std::swap(netSink, netSource);
        std::swap(netBindPath, netConnPath);
    }

    //create the bind and connect source and sink blocks
    auto bindIp = Pothos::RemoteClient::lookupIpFromNodeId(bindEnv->getNodeId());
    assert(not bindIp.empty());
    netBind = bindEnv->findProxy("Pothos/BlockRegistry").callProxy(netBindPath, "udt://"+bindIp, "BIND");
    auto connectPort = netBind.call<std::string>("getActualPort");
    auto connectUri = Poco::format("udt://%s:%s", bindIp, connectPort);
    netConn = connEnv->findProxy("Pothos/BlockRegistry").callProxy(netConnPath, connectUri, "CONNECT");

    //create the flows
    netSink.get().callVoid("setName", "NetTo: "+flow.dst.obj.call<std::string>("getName")+"["+flow.dst.name+"]");
    Flow srcFlow;
    srcFlow.src = flow.src;
    srcFlow.dst = makePort(netSink, "0");

    netSource.get().callVoid("setName", "NetFrom: "+flow.src.obj.call<std::string>("getName")+"["+flow.src.name+"]");
    Flow dstFlow;
    dstFlow.src = makePort(netSource, "0");
    dstFlow.dst = flow.dst;

    return std::make_pair(srcFlow, dstFlow);
}

/***********************************************************************
 * network crossing implementation
 **********************************************************************/
std::vector<Flow> Pothos::Topology::Impl::createNetworkFlows(const std::vector<Flow> &flatFlows)
{
    std::vector<Flow> networkAwareFlows;

    std::vector<std::pair<Flow, std::shared_future<std::pair<Flow, Flow>>>> flowToNetFlowsFuture;

    for (const auto &flow : flatFlows)
    {
        //same process, keep this flow as-is
        if (flow.src.obj.getEnvironment()->getUniquePid() == flow.dst.obj.getEnvironment()->getUniquePid())
        {
            networkAwareFlows.push_back(flow);
            continue;
        }

        //no cache entry? make a future to create new network blocks
        if (this->flowToNetgressCache.count(flow) == 0)
        {
            flowToNetFlowsFuture.push_back(
                std::make_pair(flow, std::async(std::launch::async,
                &Pothos::Topology::Impl::createNetworkFlow, this, flow)));
        }

        //lookup the network iogress flows in the cache
        else
        {
            const auto &netFlows = this->flowToNetgressCache.at(flow);
            networkAwareFlows.push_back(netFlows.first);
            networkAwareFlows.push_back(netFlows.second);
        }
    }

    //wait on netflow futures to complete and add to the cache
    for (const auto &pair : flowToNetFlowsFuture)
    {
        const auto netFlows = pair.second.get();
        flowToNetgressCache[pair.first] = netFlows;
        networkAwareFlows.push_back(netFlows.first);
        networkAwareFlows.push_back(netFlows.second);
    }

    return networkAwareFlows;
}
