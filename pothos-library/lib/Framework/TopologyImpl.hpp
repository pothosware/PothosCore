// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Framework/Topology.hpp>
#include "Framework/PortsAndFlows.hpp"
#include <unordered_map>
#include <map>
#include <vector>
#include <string>

/***********************************************************************
 * implementation guts
 **********************************************************************/
struct Pothos::Topology::Impl
{
    Impl(Topology *self): self(self){}
    Topology *self;
    ThreadPool threadPool;
    std::vector<Flow> flows;
    std::vector<Flow> activeFlatFlows;
    std::unordered_map<Flow, std::pair<Flow, Flow>> flowToNetgressCache;
    std::vector<Flow> squashFlows(const std::vector<Flow> &);
    std::vector<Flow> createNetworkFlows(const std::vector<Flow> &);
    std::vector<Flow> rectifyDomainFlows(const std::vector<Flow> &);
    std::vector<std::string> inputPortNames;
    std::vector<std::string> outputPortNames;
    std::map<std::string, PortInfo> inputPortInfo;
    std::map<std::string, PortInfo> outputPortInfo;
    std::map<std::string, Callable> calls;
};


/***********************************************************************
 * get a unique object set given flows + excludes
 **********************************************************************/
inline std::vector<Pothos::Proxy> getObjSetFromFlowList(const std::vector<Flow> &flows, const std::vector<Flow> &excludes = std::vector<Flow>())
{
    std::map<std::string, Pothos::Proxy> uniques;
    for (const auto &flow : flows)
    {
        uniques[flow.src.obj.call<std::string>("uid")] = flow.src.obj;
        uniques[flow.dst.obj.call<std::string>("uid")] = flow.dst.obj;
    }
    for (const auto &flow : excludes)
    {
        uniques.erase(flow.src.obj.call<std::string>("uid"));
        uniques.erase(flow.dst.obj.call<std::string>("uid"));
    }
    std::vector<Pothos::Proxy> set;
    for (const auto &pair : uniques) set.push_back(pair.second);
    return set;
}

/***********************************************************************
 * Make a proxy if not already
 **********************************************************************/
inline Pothos::Proxy getProxy(const Pothos::Object &o)
{
    if (o.type() == typeid(Pothos::Proxy)) return o.extract<Pothos::Proxy>();
    return Pothos::ProxyEnvironment::make("managed")->convertObjectToProxy(o);
}

/***********************************************************************
 * Avoid taking copies of self
 **********************************************************************/
inline Pothos::Proxy getInternalObject(const Pothos::Object &o, const Pothos::Topology &t)
{
    auto proxy = getProxy(o);
    if (proxy.call<std::string>("uid") == t.uid()) return Pothos::Proxy();
    return proxy;
}

/***********************************************************************
 * Want the internal block - for language bindings
 **********************************************************************/
inline Pothos::Proxy getInternalBlock(const Pothos::Proxy &block)
{
    Pothos::Proxy internal;
    try
    {
        internal = block.callProxy("getInternalBlock");
    }
    catch (const Pothos::Exception &)
    {
        internal = block;
    }
    assert(internal.getEnvironment()->getName() == "managed");
    return internal;
}

/***********************************************************************
 * Want something to make connectable calls on
 **********************************************************************/
inline Pothos::Proxy getConnectable(const Pothos::Object &o)
{
    auto proxy = getProxy(o);
    return getInternalBlock(proxy);
}

/***********************************************************************
 * can this object be connected with -- can we get a uid?
 **********************************************************************/
inline bool checkObj(const Pothos::Object &o)
{
    try
    {
        getProxy(o).call<std::string>("uid");
    }
    catch(...)
    {
        return false;
    }
    return true;
}
