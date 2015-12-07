// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/TopologyImpl.hpp"
#include <Pothos/System/HostInfo.hpp>
#include <Pothos/Remote.hpp>
#include <Poco/Net/SocketAddress.h>
#include <Poco/URI.h>
#include <future>

/***********************************************************************
 * helpers to create network iogress flows
 **********************************************************************/
std::pair<Pothos::Proxy, Pothos::Proxy> createNetworkFlow(const Flow &flow)
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
    Poco::URI uri;
    uri.setScheme("tcp");
    uri.setHost(bindIp);
    netBind = bindEnv->findProxy("Pothos/BlockRegistry").callProxy(netBindPath, uri.toString(), "BIND");
    auto connectPort = netBind.call<std::string>("getActualPort");
    uri.setPort(std::stoi(connectPort));
    netConn = connEnv->findProxy("Pothos/BlockRegistry").callProxy(netConnPath, uri.toString(), "CONNECT");

    //return the pair of network blocks
    const auto name = flow.src.obj.call<std::string>("getName")+"["+flow.src.name+"]";
    netSink.get().callVoid("setName", "NetTo: "+name);
    netSource.get().callVoid("setName", "NetFrom: "+name);
    return std::make_pair(netSource, netSink);
}

/***********************************************************************
 * network crossing implementation
 **********************************************************************/
std::vector<Flow> Pothos::Topology::Impl::createNetworkFlows(const std::vector<Flow> &flatFlows)
{
    std::vector<Flow> networkAwareFlows;

    //locate all of the source endpoints
    std::unordered_map<Port, std::vector<Flow>> srcToFlows;
    for (const auto &flow : flatFlows)
    {
        //same process, keep this flow as-is
        if (flow.src.obj.getEnvironment()->getUniquePid() == flow.dst.obj.getEnvironment()->getUniquePid())
        {
            networkAwareFlows.push_back(flow);
            continue;
        }
        srcToFlows[envTagPort(flow.src, flow.dst)].push_back(flow);
    }
    //look in the cache or create network iogress for every source endpoint
    std::unordered_map<Port, std::shared_future<std::pair<Pothos::Proxy, Pothos::Proxy>>> srcToFutures;
    for (const auto &pair : srcToFlows)
    {
        assert(not pair.second.empty());
        if (this->srcToNetgressCache.count(pair.first) != 0) continue;
        srcToFutures[pair.first] = std::async(std::launch::async, &createNetworkFlow, pair.second.at(0));
    }

    //load all futures into the cache
    for (const auto &pair : srcToFutures)
    {
        this->srcToNetgressCache[pair.first] = pair.second.get();
    }

    //append network flows from the cache
    for (const auto &pair : srcToFlows)
    {
        assert(not pair.second.empty());
        const auto &netBlocks = this->srcToNetgressCache.at(pair.first);

        //append the source to netSink flow
        Flow srcFlow;
        srcFlow.src = pair.second.at(0).src;
        srcFlow.dst = makePort(netBlocks.second, "0");
        networkAwareFlows.push_back(srcFlow);

        //append the netSource to dest flows
        for (const auto &flow : pair.second)
        {
            assert(flow.src == srcFlow.src);
            Flow dstFlow;
            dstFlow.src = makePort(netBlocks.first, "0");
            dstFlow.dst = flow.dst;
            networkAwareFlows.push_back(dstFlow);
        }
    }

    return networkAwareFlows;
}
