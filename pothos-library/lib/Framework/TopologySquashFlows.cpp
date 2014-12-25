// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/TopologyImpl.hpp"
#include <iostream>
#include <future>

/***********************************************************************
 * helpers to deal with recursive topology comprehension - ports
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
            if (flow.src.obj) subPorts = resolvePorts(flow.src, isSource);
            else ports.push_back(flow.src);
        }
        if (not isSource and flow.src.name == portName and not flow.src.obj)
        {
            if (flow.dst.obj) subPorts = resolvePorts(flow.dst, isSource);
            else ports.push_back(flow.dst);
        }
        ports.insert(ports.end(), subPorts.begin(), subPorts.end());
    }
    return ports;
}

static Port proxyToPort(const Pothos::Proxy &portProxy)
{
    Port port;
    port.name = portProxy.call<std::string>("get:name");
    port.obj = portProxy.callProxy("get:obj");
    port.uid = portProxy.call<std::string>("get:uid");
    return port;
}

static std::vector<Port> resolvePorts(const Port &port, const bool isSource)
{
    std::vector<Port> ports;

    //resolve ports connected to the topology
    Pothos::Proxy subPorts;
    try
    {
        subPorts = port.obj.callProxy("resolvePorts", port.name, isSource);
    }
    catch (const Pothos::Exception &)
    {
        //its just a block, no ports to resolve
        ports.push_back(port);
        return ports;
    }

    const auto len = subPorts.call<size_t>("size");
    for (size_t i = 0; i < len; i++)
    {
        ports.push_back(proxyToPort(subPorts.callProxy("at", i)));
    }

    return ports;
}

/***********************************************************************
 * helpers to deal with recursive topology comprehension - flows
 **********************************************************************/
std::vector<Flow> resolveFlowsFromTopology(const Pothos::Topology &t)
{
    return t._impl->squashFlows(t._impl->flows);
}

static Flow proxyToFlow(const Pothos::Proxy &flowProxy)
{
    Flow flow;
    flow.src = proxyToPort(flowProxy.callProxy("get:src"));
    flow.dst = proxyToPort(flowProxy.callProxy("get:dst"));
    return flow;
}

static std::vector<Flow> resolveFlows(const Pothos::Proxy &obj)
{
    std::vector<Flow> flows;

    //resolve flows within the topology
    Pothos::Proxy subFlows;
    try
    {
        subFlows = obj.callProxy("resolveFlows");
    }
    catch (const Pothos::Exception &)
    {
        return flows;
    }

    const auto len = subFlows.call<size_t>("size");
    for (size_t i = 0; i < len; i++)
    {
        flows.push_back(proxyToFlow(subFlows.callProxy("at", i)));
    }

    return flows;
}

/***********************************************************************
 * complete pass-through flows
 **********************************************************************/
static std::vector<Flow> completeFlows(const std::vector<Flow> &flows)
{
    std::vector<Flow> completeFlows;
    for (auto &flow : flows)
    {
        //its a complete flow
        if (flow.src.obj and flow.dst.obj)
        {
            completeFlows.push_back(flow);
        }
        //find all complete flow matches from a lower level pass-through flow
        if (not flow.src.obj and not flow.dst.obj)
        {
            for (auto &flowTail : flows)
            {
                if (not flowTail.dst.obj) continue;
                for (auto &flowHead : flows)
                {
                    if (not flowHead.src.obj) continue;
                    if (flow.src == flowTail.src and flow.dst == flowHead.dst)
                    {
                        Flow newFlow;
                        newFlow.src = flowHead.src;
                        newFlow.dst = flowTail.dst;
                        completeFlows.push_back(newFlow);
                    }
                }
            }
        }
    }
    return completeFlows;
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

    //get a list of objects
    std::map<std::string, Pothos::Proxy> uidToObj;
    for (const auto &flow : flows)
    {
        if (flow.src.obj) uidToObj[flow.src.uid] = flow.src.obj;
        if (flow.dst.obj) uidToObj[flow.dst.uid] = flow.dst.obj;
    }

    //spawn futures to resolve sub-topology flows
    std::vector<std::shared_future<std::vector<Flow>>> futureFlows;
    for (const auto &pair : uidToObj)
    {
        futureFlows.push_back(std::async(std::launch::async, &resolveFlows, pair.second));
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
    for (const auto &futureFlow : futureFlows)
    {
        const auto flows = futureFlow.get();
        flatFlows.insert(flatFlows.end(), flows.begin(), flows.end());
    }

    //squash pass-through blocks
    //complete flows can only contain flows between real blocks
    flatFlows = completeFlows(flatFlows);

    //only store the actual blocks
    for (auto &flow : flatFlows)
    {
        flow.src.obj = getInternalBlock(flow.src.obj);
        flow.dst.obj = getInternalBlock(flow.dst.obj);
    }

    //collect flows that pass through this topology in -> out
    //the outer topology will squash the pass-through flows
    for (const auto &flow : flows)
    {
        if (not flow.src.obj and not flow.dst.obj) flatFlows.push_back(flow);
    }

    return flatFlows;
}
