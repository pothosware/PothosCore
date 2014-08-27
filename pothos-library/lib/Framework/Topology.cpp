// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/TopologyImpl.hpp"
#include <Pothos/Framework/Block.hpp>
#include <Pothos/Framework/Exception.hpp>
#include <Pothos/Object.hpp>
#include <Pothos/Proxy.hpp>
#include <Poco/Logger.h>
#include <Poco/Format.h>
#include <algorithm>
#include <cassert>
#include <set>
#include <cctype>
#include <chrono>
#include <thread>
#include <iostream>

/***********************************************************************
 * Topology implementation
 **********************************************************************/
std::shared_ptr<Pothos::Topology> Pothos::Topology::make(void)
{
    return std::make_shared<Topology>();
}

Pothos::Topology::Topology(void):
    _impl(new Impl(this))
{
    return;
}

Pothos::Topology::~Topology(void)
{
    std::cout << "~Topology\n";
    try
    {
        this->disconnectAll();
        this->commit();
        assert(this->_impl->activeFlatFlows.empty());
        //assert(this->_impl->flowToNetgressCache.empty());
    }
    catch (const Pothos::Exception &ex)
    {
        poco_error_f1(Poco::Logger::get("Pothos.Topology"), "Topology destructor threw: %s", ex.displayText());
    }
}

Port Pothos::Topology::Impl::makePort(const Pothos::Object &obj, const std::string &name) const
{
    return this->makePort(getProxy(obj), name);
}

Port Pothos::Topology::Impl::makePort(const Pothos::Proxy &obj, const std::string &name) const
{
    Port p;
    p.name = name;
    p.uid = obj.call<std::string>("uid");
    //dont store copies of self
    if (p.uid != self->uid()) p.obj = obj;
    return p;
}

void Pothos::Topology::setThreadPool(const ThreadPool &threadPool)
{
    _impl->threadPool = threadPool;
}

const Pothos::ThreadPool &Pothos::Topology::getThreadPool(void) const
{
    return _impl->threadPool;
}

std::vector<Pothos::PortInfo> Pothos::Topology::inputPortInfo(void)
{
    std::vector<PortInfo> infos;
    std::set<std::string> names;
    for (const auto &name : _impl->inputPortNames)
    {
        if (names.count(name) != 0) continue;
        names.insert(name);
        infos.push_back(_impl->inputPortInfo.at(name));
        assert(_impl->inputPortInfo.at(name).name == name);
    }
    return infos;
}

std::vector<Pothos::PortInfo> Pothos::Topology::outputPortInfo(void)
{
    std::vector<PortInfo> infos;
    std::set<std::string> names;
    for (const auto &name : _impl->outputPortNames)
    {
        if (names.count(name) != 0) continue;
        names.insert(name);
        infos.push_back(_impl->outputPortInfo.at(name));
        assert(_impl->outputPortInfo.at(name).name == name);
    }
    return infos;
}

void Pothos::Topology::_connect(
    const Object &src, const std::string &srcName,
    const Object &dst, const std::string &dstName)
{
    if (not checkObj(src)) throw Pothos::TopologyConnectError("Pothos::Topology::connect()",
        "source port of type " + src.toString());
    if (not checkObj(dst)) throw Pothos::TopologyConnectError("Pothos::Topology::connect()",
        "destination port of type " + dst.toString());

    Flow flow;
    flow.src = _impl->makePort(src, srcName);
    flow.dst = _impl->makePort(dst, dstName);

    //perform auto-allocation, on a block this may or may not allocate, on a topology this throws
    try{getConnectable(src).callProxy("get:_actor").call<std::string>("autoAllocateOutput", srcName);}catch(const Exception &){}
    try{getConnectable(dst).callProxy("get:_actor").call<std::string>("autoAllocateInput", dstName);}catch(const Exception &){}

    //validate that the ports exists before connection
    if (flow.src.obj)
    {
        auto outs = getConnectable(src).call<std::vector<std::string>>("outputPortNames");
        if (std::find(outs.begin(), outs.end(), srcName) == outs.end())
            throw Pothos::TopologyConnectError("Pothos::Topology::connect()", src.toString() + " has no output port named " + srcName);
    }

    if (flow.dst.obj)
    {
        auto ins = getConnectable(dst).call<std::vector<std::string>>("inputPortNames");
        if (std::find(ins.begin(), ins.end(), dstName) == ins.end())
            throw Pothos::TopologyConnectError("Pothos::Topology::connect()", dst.toString() + " has no input port named " + dstName);
    }

    //store port info for connections to the hierachy
    if (not flow.src.obj)
    {
        _impl->inputPortNames.push_back(srcName);
        for (const auto &info : getConnectable(dst).call<std::vector<PortInfo>>("inputPortInfo"))
        {
            if (info.name == dstName)
            {
                _impl->inputPortInfo[srcName] = info;
                _impl->inputPortInfo[srcName].name = srcName;
            }
        }
    }
    if (not flow.dst.obj)
    {
        _impl->outputPortNames.push_back(dstName);
        for (const auto &info : getConnectable(src).call<std::vector<PortInfo>>("outputPortInfo"))
        {
            if (info.name == srcName)
            {
                _impl->outputPortInfo[dstName] = info;
                _impl->outputPortInfo[dstName].name = dstName;
            }
        }
    }

    const auto it = std::find(_impl->flows.begin(), _impl->flows.end(), flow);
    if (it != _impl->flows.end()) throw Pothos::TopologyConnectError("Pothos::Topology::connect()",
        "this flow already exists in the topology");

    _impl->flows.push_back(flow);
}

void Pothos::Topology::_disconnect(
    const Object &src, const std::string &srcName,
    const Object &dst, const std::string &dstName)
{
    if (not checkObj(src)) throw Pothos::TopologyConnectError("Pothos::Topology::disconnect()",
        "source port of type " + src.toString());
    if (not checkObj(dst)) throw Pothos::TopologyConnectError("Pothos::Topology::disconnect()",
        "destination port of type " + dst.toString());

    Flow flow;
    flow.src = _impl->makePort(src, srcName);
    flow.dst = _impl->makePort(dst, dstName);

    //validate that the ports exists before disconnection
    auto outs = getConnectable(src).call<std::vector<std::string>>("outputPortNames");
    if (std::find(outs.begin(), outs.end(), srcName) == outs.end())
        throw Pothos::TopologyConnectError("Pothos::Topology::disconnect()", src.toString() + " has no output port named " + srcName);

    auto ins = getConnectable(dst).call<std::vector<std::string>>("inputPortNames");
    if (std::find(ins.begin(), ins.end(), dstName) == ins.end())
        throw Pothos::TopologyConnectError("Pothos::Topology::disconnect()", dst.toString() + " has no input port named " + dstName);

    //clear port info for disconnections from the hierachy
    if (not flow.src.obj) _impl->outputPortNames.erase(std::find(_impl->outputPortNames.begin(), _impl->outputPortNames.end(), srcName));
    if (not flow.dst.obj) _impl->inputPortNames.erase(std::find(_impl->inputPortNames.begin(), _impl->inputPortNames.end(), dstName));

    const auto it = std::find(_impl->flows.begin(), _impl->flows.end(), flow);
    if (it == _impl->flows.end()) throw Pothos::TopologyConnectError("Pothos::Topology::disconnect()",
        "this flow does not exist in the topology");

    _impl->flows.erase(it);
}

void Pothos::Topology::disconnectAll(void)
{
    //call disconnect all on the sub-topologies
    for (const auto &flow : _impl->flows)
    {
        //throws ProxyHandleCallError on non topologies (aka blocks)
        if (flow.src.obj) try
        {
            flow.src.obj.callVoid("disconnectAll");
        }
        catch (const Pothos::Exception &){}

        if (flow.dst.obj) try
        {
            flow.dst.obj.callVoid("disconnectAll");
        }
        catch (const Pothos::Exception &){}
    }

    //clear our own local flows
    _impl->flows.clear();
}

bool Pothos::Topology::waitInactive(const double idleDuration, const double timeout)
{
    //how long to sleep between idle checks?
    const std::chrono::nanoseconds idleDurationNs((long long)(idleDuration*1e9));
    const std::chrono::nanoseconds pollSleepTime(idleDurationNs/3);

    //get a list of blocks to poll for idle time
    const auto blocks = getObjSetFromFlowList(_impl->activeFlatFlows);

    //loop until exit time
    const auto exitTime = std::chrono::high_resolution_clock::now() + std::chrono::nanoseconds((long long)(timeout*1e9));
    do
    {
        //check each worker for idle time from the stats
        for (auto block : blocks)
        {
            const auto stats = block.call<WorkStats>("workStats");
            const auto consumptionIdle = stats.timeStatsQuery - stats.timeLastConsumed;
            const auto productionIdle = stats.timeStatsQuery - stats.timeLastProduced;
            const auto workerIdleDuration = std::min(consumptionIdle, productionIdle);
            if (workerIdleDuration < idleDurationNs) goto pollSleep;
        }

        //all workers reached the max idle time specified
        return true;

        //idle time not reached on any workers, therefore sleep
        pollSleep: std::this_thread::sleep_for(pollSleepTime);
    }
    while (std::chrono::high_resolution_clock::now() < exitTime);

    return false; //timeout
}

void Pothos::Topology::registerCallable(const std::string &name, const Callable &call)
{
    _impl->calls[name] = call;
}

Pothos::Object Pothos::Topology::opaqueCallMethod(const std::string &name, const Object *inputArgs, const size_t numArgs) const
{
    return _impl->self->opaqueCallHandler(name, inputArgs, numArgs);
}

Pothos::Object Pothos::Topology::opaqueCallHandler(const std::string &name, const Object *inputArgs, const size_t numArgs)
{
    auto it = _impl->calls.find(name);
    if (it == _impl->calls.end())
    {
        throw Pothos::BlockCallNotFound("Pothos::Topology::call("+name+")", "method does not exist in registry");
    }
    return it->second.opaqueCall(inputArgs, numArgs);
}

#include <Pothos/Managed.hpp>

static Pothos::ProxyVector getFlowsFromTopology(const Pothos::Topology &t)
{
    Pothos::ProxyVector flows;
    for (const auto &flow : t._impl->flows)
    {
        flows.push_back(Pothos::ProxyEnvironment::make("managed")->makeProxy(flow));
    }
    return flows;
}

std::vector<Port> resolvePortsFromTopology(const Pothos::Topology &t, const std::string &portName, const bool isSource);

static auto managedTopology = Pothos::ManagedClass()
    .registerClass<Pothos::Topology>()
    .registerBaseClass<Pothos::Topology, Pothos::Connectable>()
    .registerStaticMethod(POTHOS_FCN_TUPLE(Pothos::Topology, make))
    .registerMethod("getFlows", &getFlowsFromTopology)
    .registerMethod("resolvePorts", &resolvePortsFromTopology)
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Topology, setThreadPool))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Topology, getThreadPool))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Topology, commit))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Topology, disconnectAll))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Topology, waitInactive))
    //and bind defaults into waitInactive for optional trailing arguments
    .registerMethod("waitInactive", Pothos::Callable(&Pothos::Topology::waitInactive).bind(1.0, 2))
    .registerMethod("waitInactive", Pothos::Callable(&Pothos::Topology::waitInactive).bind(1.0, 2).bind(0.1, 1))
    .registerMethod("connect", &Pothos::Topology::_connect)
    .registerMethod("disconnect", &Pothos::Topology::_disconnect)
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Topology, toDotMarkup))
    .commit("Pothos/Topology");

static auto managedPort = Pothos::ManagedClass()
    .registerClass<Port>()
    .registerField(POTHOS_FCN_TUPLE(Port, obj))
    .registerField(POTHOS_FCN_TUPLE(Port, name))
    .registerField(POTHOS_FCN_TUPLE(Port, uid))
    .commit("Pothos/Topology/Port");

static size_t portVectorSize(const std::vector<Port> &vec)
{
    return vec.size();
}

static Port portVectorAt(const std::vector<Port> &vec, const size_t index)
{
    return vec.at(index);
}

static auto managedPortVector = Pothos::ManagedClass()
    .registerClass<std::vector<Port>>()
    .registerMethod("size", &portVectorSize)
    .registerMethod("at", &portVectorAt)
    .commit("Pothos/Topology/PortVector");

static auto managedFlow = Pothos::ManagedClass()
    .registerClass<Flow>()
    .registerField(POTHOS_FCN_TUPLE(Flow, src))
    .registerField(POTHOS_FCN_TUPLE(Flow, dst))
    .commit("Pothos/Topology/Flow");
