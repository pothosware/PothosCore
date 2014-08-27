// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/TopologyImpl.hpp"

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

std::vector<Flow> Pothos::Topology::Impl::squashFlows(const std::vector<Flow> &flows)
{
    std::vector<Flow> flatFlows;

    for (const auto &flow : flows)
    {
        //ignore external flows
        if (not flow.src.obj) continue;
        if (not flow.dst.obj) continue;

        //gather a list of sources and destinations on either end of this flow
        std::vector<Port> srcs = resolvePorts(flow.src, true);
        std::vector<Port> dsts = resolvePorts(flow.dst, false);

        //all combinations of srcs + dsts are flows
        for (const auto &src : srcs)
        {
            for (const auto &dst : dsts)
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
