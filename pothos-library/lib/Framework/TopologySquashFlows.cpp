// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/TopologyImpl.hpp"
#include <future>

/***********************************************************************
 * helpers to deal with recursive topology comprehension
 **********************************************************************/
static std::vector<Port> resolvePorts(const Port &port, const bool isSource);

std::vector<Port> resolvePortsFromTopology(const Pothos::Topology &t, const std::string &portName, const bool isSource)
{
    std::vector<Port> ports;
    for (const auto &flow : t._impl->flows)
    {
        //recurse through sub topology flows
        std::vector<Port> subPorts;
        if (isSource and flow.dst.name == portName and not flow.dst.obj)
        {
            subPorts = resolvePorts(flow.src, isSource);
        }
        if (not isSource and flow.src.name == portName and not flow.src.obj)
        {
            subPorts = resolvePorts(flow.dst, isSource);
        }
        ports.insert(ports.end(), subPorts.begin(), subPorts.end());
    }
    return ports;
}

static std::vector<Port> resolvePorts(const Port &port, const bool isSource)
{
    std::vector<Port> ports;

    //resolve ports connected to the topology
    try
    {
        auto subPorts = port.obj.callProxy("resolvePorts", port.name, isSource);
        for (size_t i = 0; i < subPorts.call<size_t>("size"); i++)
        {
            auto portProxy = subPorts.callProxy("at", i);
            Port port;
            port.name = portProxy.call<std::string>("get:name");
            port.obj = portProxy.callProxy("get:obj");
            port.uid = portProxy.call<std::string>("get:uid");
            ports.push_back(port);
        }
    }
    catch (const Pothos::Exception &)
    {
        //its just a block, no ports to resolve
        ports.push_back(port);
    }

    return ports;
}

/***********************************************************************
 * topology squash implementation
 **********************************************************************/
std::vector<Flow> Pothos::Topology::Impl::squashFlows(const std::vector<Flow> &flows)
{
    //spawn future to resolve ports per flow
    std::vector<std::shared_future<std::vector<Port>>> future_srcs, future_dsts;
    for (const auto &flow : flows)
    {
        //ignore external flows
        if (not flow.src.obj) continue;
        if (not flow.dst.obj) continue;

        //gather a list of sources and destinations on either end of this flow
        future_srcs.push_back(std::async(std::launch::async, &resolvePorts, flow.src, true));
        future_dsts.push_back(std::async(std::launch::async, &resolvePorts, flow.dst, false));
    }

    //create flat flows from futures
    std::vector<Flow> flatFlows;
    assert(future_srcs.size() == future_dsts.size());
    for (size_t i = 0; i < future_srcs.size(); i++)
    {
        //all combinations of srcs + dsts are flows
        for (const auto &src : future_srcs[i].get())
        {
            for (const auto &dst : future_dsts[i].get())
            {
                Flow flatFlow;
                flatFlow.src = src;
                flatFlow.dst = dst;
                flatFlows.push_back(flatFlow);
            }
        }
    }

    //only store the actual blocks
    for (auto &flow : flatFlows)
    {
        flow.src.obj = getInternalBlock(flow.src.obj);
        flow.dst.obj = getInternalBlock(flow.dst.obj);
    }

    return flatFlows;
}
