// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/TopologyImpl.hpp"
#include <Pothos/System/HostInfo.hpp>
#include <Pothos/Remote.hpp>
#include <Poco/Format.h>

/***********************************************************************
 * helpers to create network iogress flows
 **********************************************************************/
std::vector<Flow> Pothos::Topology::Impl::createNetworkFlows(const std::vector<Flow> &flatFlows)
{
    std::vector<Flow> networkAwareFlows;
    for (const auto &flow : flatFlows)
    {
        //same process, keep this flow as-is
        if (flow.src.obj.getEnvironment()->getUniquePid() == flow.dst.obj.getEnvironment()->getUniquePid())
        {
            networkAwareFlows.push_back(flow);
            continue;
        }

        //lookup the network iogress flows in the cache
        auto it = this->flowToNetgressCache.find(flow);
        if (it != this->flowToNetgressCache.end())
        {
            networkAwareFlows.push_back(it->second.first);
            networkAwareFlows.push_back(it->second.second);
        }

        //otherwise make new network iogress flows
        else
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

            //add the network flows to the overall list
            networkAwareFlows.push_back(srcFlow);
            networkAwareFlows.push_back(dstFlow);

            //save it in the cache
            flowToNetgressCache[flow] = std::make_pair(srcFlow, dstFlow);
        }
    }

    return networkAwareFlows;
}
