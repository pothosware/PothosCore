// Copyright (c) 2014-2019 Josh Blum
//                    2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include "Framework/TopologyImpl.hpp"
#include <Pothos/Framework/Block.hpp>
#include <Pothos/Framework/Exception.hpp>
#include <Pothos/Object.hpp>
#include <Pothos/Plugin.hpp>
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
    try
    {
        this->disconnectAll();
        this->commit();
        assert(this->_impl->activeFlatFlows.empty());
        assert(this->_impl->srcToNetgressCache.empty());
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
    p.objName = obj.call<std::string>("getName");
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

void Pothos::Topology::setInputAlias(const std::string &portName, const std::string &alias)
{
    if (_impl->inputPortInfo.count(portName) == 0) throw PortAccessError(
        "Pothos::Topology::setInputAlias("+portName+")", "unknown port");
    _impl->inputPortInfo[portName].alias = alias;
}

void Pothos::Topology::setOutputAlias(const std::string &portName, const std::string &alias)
{
    if (_impl->outputPortInfo.count(portName) == 0) throw PortAccessError(
        "Pothos::Topology::setOutputAlias("+portName+")", "unknown port");
    _impl->outputPortInfo[portName].alias = alias;
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
    try{getConnectable(src).get("_actor").call("autoAllocateOutput", srcName);}catch(const Exception &){}
    try{getConnectable(dst).get("_actor").call("autoAllocateInput", dstName);}catch(const Exception &){}

    //validate that the ports exists before connection
    if (flow.src.obj)
    {
        std::vector<std::string> outs = getConnectable(src).call("outputPortNames");
        if (std::find(outs.begin(), outs.end(), srcName) == outs.end())
            throw Pothos::TopologyConnectError("Pothos::Topology::connect()", flow.src.toString() + " has no output port named " + srcName);
    }

    if (flow.dst.obj)
    {
        std::vector<std::string> ins = getConnectable(dst).call("inputPortNames");
        if (std::find(ins.begin(), ins.end(), dstName) == ins.end())
            throw Pothos::TopologyConnectError("Pothos::Topology::connect()", flow.dst.toString() + " has no input port named " + dstName);
    }

    //store port info for connections to the hierachy
    if (not flow.src.obj and flow.dst.obj)
    {
        _impl->inputPortNames.push_back(srcName);
        for (const auto &info : getConnectable(dst).call<std::vector<PortInfo>>("inputPortInfo"))
        {
            if (info.name == dstName)
            {
                auto &newInfo = _impl->inputPortInfo[srcName];
                newInfo.name = srcName;
                newInfo.alias = srcName;
                newInfo.isSigSlot = info.isSigSlot;
                newInfo.dtype = info.dtype;
            }
        }
    }
    if (not flow.dst.obj and flow.src.obj)
    {
        _impl->outputPortNames.push_back(dstName);
        for (const auto &info : getConnectable(src).call<std::vector<PortInfo>>("outputPortInfo"))
        {
            if (info.name == srcName)
            {
                auto &newInfo = _impl->outputPortInfo[dstName];
                newInfo.name = dstName;
                newInfo.alias = dstName;
                newInfo.isSigSlot = info.isSigSlot;
                newInfo.dtype = info.dtype;
            }
        }
    }
    if (not flow.src.obj and not flow.dst.obj)
    {
        _impl->inputPortNames.push_back(srcName);
        _impl->inputPortInfo[srcName].name = srcName;

        _impl->outputPortNames.push_back(dstName);
        _impl->outputPortInfo[dstName].name = dstName;
    }

    const auto it = std::find(_impl->flows.begin(), _impl->flows.end(), flow);
    if (it != _impl->flows.end()) throw Pothos::TopologyConnectError("Pothos::Topology::connect()",
        Poco::format("this flow already exists in the topology(%s)", flow.toString()));

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
    std::vector<std::string> outs = getConnectable(src).call("outputPortNames");
    if (std::find(outs.begin(), outs.end(), srcName) == outs.end())
        throw Pothos::TopologyConnectError("Pothos::Topology::disconnect()", flow.src.toString() + " has no output port named " + srcName);

    std::vector<std::string> ins = getConnectable(dst).call("inputPortNames");
    if (std::find(ins.begin(), ins.end(), dstName) == ins.end())
        throw Pothos::TopologyConnectError("Pothos::Topology::disconnect()", flow.dst.toString() + " has no input port named " + dstName);

    //clear port info for disconnections from the hierachy
    if (not flow.src.obj) _impl->outputPortNames.erase(std::find(_impl->outputPortNames.begin(), _impl->outputPortNames.end(), srcName));
    if (not flow.dst.obj) _impl->inputPortNames.erase(std::find(_impl->inputPortNames.begin(), _impl->inputPortNames.end(), dstName));

    const auto it = std::find(_impl->flows.begin(), _impl->flows.end(), flow);
    if (it == _impl->flows.end()) throw Pothos::TopologyConnectError("Pothos::Topology::disconnect()",
        Poco::format("this flow does not exist in the topology(%s)", flow.toString()));

    //perform auto-deletion, on a block this may or may not delete, on a topology this throws
    try{getConnectable(src).get("_actor").call("autoDeleteOutput", srcName);}catch(const Exception &){}
    try{getConnectable(dst).get("_actor").call("autoDeleteInput", dstName);}catch(const Exception &){}

    _impl->flows.erase(it);
}

void Pothos::Topology::disconnectAll(const bool recursive)
{
    //call disconnect all on the sub-topologies
    if (recursive) for (const auto &flow : _impl->flows)
    {
        //throws ProxyHandleCallError on non topologies (aka blocks)
        if (flow.src.obj) try
        {
            flow.src.obj.call("disconnectAll");
        }
        catch (const Pothos::Exception &){}

        if (flow.dst.obj) try
        {
            flow.dst.obj.call("disconnectAll");
        }
        catch (const Pothos::Exception &){}
    }

    //perform auto-deletion, on a block this may or may not delete, on a topology this throws
    for (const auto &flow : _impl->flows)
    {
        if (flow.src.obj) try{getInternalBlock(flow.src.obj).get("_actor").call("autoDeleteOutput", flow.src.name);}catch(const Exception &){}
        if (flow.dst.obj) try{getInternalBlock(flow.dst.obj).get("_actor").call("autoDeleteInput", flow.dst.name);}catch(const Exception &){}
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

    //track block activity indicators and activity time stamps
    std::vector<std::chrono::high_resolution_clock::time_point> lastActivityTime(blocks.size());
    std::vector<int> lastActivityIndicator(blocks.size(), 0);

    //loop until exit time
    const auto entryTime = std::chrono::high_resolution_clock::now();
    const auto exitTime = entryTime + std::chrono::nanoseconds((long long)(timeout*1e9));
    do
    {
        //check each worker for idle time from the activity indicator
        bool allIdle = true;
        for (size_t i = 0; i < blocks.size(); i++)
        {
            const auto &block = blocks[i];
            const int activityIndicator = block.get("_actor").call("queryActivityIndicator");
            if (lastActivityIndicator[i] != activityIndicator)
            {
                lastActivityTime[i] = std::chrono::high_resolution_clock::now();
                lastActivityIndicator[i] = activityIndicator;
            }
            const auto workerIdleDuration = std::chrono::high_resolution_clock::now() - std::max(entryTime, lastActivityTime[i]);
            if (workerIdleDuration < idleDurationNs) allIdle = false;
        }

        //all workers reached the max idle time specified
        if (allIdle) return true;

        //idle time not reached on any workers, therefore sleep
        else std::this_thread::sleep_for(pollSleepTime);
    }
    while (std::chrono::high_resolution_clock::now() < exitTime or timeout == 0.0);

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
std::vector<Flow> resolveFlowsFromTopology(const Pothos::Topology &t);
void topologySubCommit(Pothos::Topology &topology);

static auto managedTopology = Pothos::ManagedClass()
    .registerClass<Pothos::Topology>()
    .registerBaseClass<Pothos::Topology, Pothos::Connectable>()
    .registerStaticMethod("make", (std::shared_ptr<Pothos::Topology>(*)(void))&Pothos::Topology::make)
    .registerStaticMethod<std::shared_ptr<Pothos::Topology>, const std::string &>(POTHOS_FCN_TUPLE(Pothos::Topology, make))
    .registerMethod("getFlows", &getFlowsFromTopology)
    .registerMethod("subCommit", &topologySubCommit)
    .registerMethod("resolvePorts", &resolvePortsFromTopology)
    .registerMethod("resolveFlows", &resolveFlowsFromTopology)
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Topology, setThreadPool))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Topology, getThreadPool))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Topology, commit))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Topology, disconnectAll))
    .registerMethod("disconnectAll", Pothos::Callable(&Pothos::Topology::disconnectAll).bind(false, 1))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Topology, waitInactive))
    //and bind defaults into waitInactive for optional trailing arguments
    .registerMethod("waitInactive", Pothos::Callable(&Pothos::Topology::waitInactive).bind(1.0, 2))
    .registerMethod("waitInactive", Pothos::Callable(&Pothos::Topology::waitInactive).bind(1.0, 2).bind(0.1, 1))
    .registerMethod("connect", &Pothos::Topology::_connect)
    .registerMethod("disconnect", &Pothos::Topology::_disconnect)
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Topology, toDotMarkup))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Topology, queryJSONStats))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Topology, dumpJSON))
    .commit("Pothos/Topology");

/***********************************************************************
 * port type registration
 **********************************************************************/
static auto managedPort = Pothos::ManagedClass()
    .registerClass<Port>()
    .registerField(POTHOS_FCN_TUPLE(Port, obj))
    .registerField(POTHOS_FCN_TUPLE(Port, name))
    .registerField(POTHOS_FCN_TUPLE(Port, uid))
    .registerField(POTHOS_FCN_TUPLE(Port, objName))
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

/***********************************************************************
 * flow type registration
 **********************************************************************/
static auto managedFlow = Pothos::ManagedClass()
    .registerClass<Flow>()
    .registerField(POTHOS_FCN_TUPLE(Flow, src))
    .registerField(POTHOS_FCN_TUPLE(Flow, dst))
    .commit("Pothos/Topology/Flow");

static size_t flowVectorSize(const std::vector<Flow> &vec)
{
    return vec.size();
}

static Flow flowVectorAt(const std::vector<Flow> &vec, const size_t index)
{
    return vec.at(index);
}

static auto managedFlowVector = Pothos::ManagedClass()
    .registerClass<std::vector<Flow>>()
    .registerMethod("size", &flowVectorSize)
    .registerMethod("at", &flowVectorAt)
    .commit("Pothos/Topology/FlowVector");

/***********************************************************************
 * Register toString() outputs
 **********************************************************************/

#include "Object/ToString.hpp"

static std::string pothosTopologyToString(const Pothos::Topology& topology)
{
    return "Pothos::Topology (name: " + topology.getName() + ")";
}

pothos_static_block(pothosRegisterTopologyToString)
{
    Pothos::PluginRegistry::addCall(
        "/object/tostring/Pothos/Topology",
        Pothos::Callable(&pothosTopologyToString));
    Pothos::PluginRegistry::addCall(
        "/object/tostring/Pothos/TopologySPtr",
        Pothos::Callable(&sptrObjectToString<Pothos::Topology>));
}
