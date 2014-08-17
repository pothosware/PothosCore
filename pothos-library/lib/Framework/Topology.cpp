// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/TopologyImpl.hpp"
#include <Pothos/Framework/Block.hpp>
#include <Pothos/Framework/Exception.hpp>
#include <Pothos/Remote.hpp>
#include <Pothos/Object.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/System/HostInfo.hpp>
#include <Poco/Environment.h>
#include <Poco/Logger.h>
#include <Poco/Format.h>
#include <Poco/Timestamp.h>
#include <Poco/Timespan.h>
#include <algorithm>
#include <cassert>
#include <set>
#include <cctype>
#include <chrono>
#include <thread>
#include <iostream>

/***********************************************************************
 * Make a proxy if not already
 **********************************************************************/
static Pothos::Proxy getProxy(const Pothos::Object &o)
{
    if (o.type() == typeid(Pothos::Proxy)) return o.extract<Pothos::Proxy>();
    return Pothos::ProxyEnvironment::make("managed")->convertObjectToProxy(o);
}

/***********************************************************************
 * Avoid taking copies of self
 **********************************************************************/
static Pothos::Proxy getInternalObject(const Pothos::Object &o, const Pothos::Topology &t)
{
    auto proxy = getProxy(o);
    if (proxy.call<std::string>("uid") == t.uid()) return Pothos::Proxy();
    return proxy;
}

/***********************************************************************
 * Want the internal block - for language bindings
 **********************************************************************/
static Pothos::Proxy getInternalBlock(const Pothos::Proxy &block)
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
static Pothos::Proxy getConnectable(const Pothos::Object &o)
{
    auto proxy = getProxy(o);
    return getInternalBlock(proxy);
}

/***********************************************************************
 * can this object be connected with -- can we get a uid?
 **********************************************************************/
static bool checkObj(const Pothos::Object &o)
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

/***********************************************************************
 * helpers to deal with recursive topology comprehension
 **********************************************************************/
static std::vector<Port> resolvePorts(const Port &port, const bool isSource);

static std::vector<Port> resolvePortsFromTopology(const Pothos::Topology &t, const std::string &portName, const bool isSource)
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

static std::vector<Flow> squashFlows(const std::vector<Flow> &flows)
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
            srcFlow.dst.obj = netSink;
            srcFlow.dst.name = "0";

            netSource.get().callVoid("setName", "NetFrom: "+flow.src.obj.call<std::string>("getName")+"["+flow.src.name+"]");
            Flow dstFlow;
            dstFlow.src.obj = netSource;
            dstFlow.src.name = "0";
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

/***********************************************************************
 * helpers to deal with domain interaction
 **********************************************************************/
static bool isDomainCrossingAcceptable(
    const Port &mainPort,
    const std::vector<Port> &subPorts,
    const bool isInput
)
{
    auto mainDomain = mainPort.obj.callProxy(isInput?"input":"output", mainPort.name).call<std::string>("domain");

    bool allOthersAbdicate = true;
    std::set<std::string> subDomains;
    for (const auto &subPort : subPorts)
    {
        subDomains.insert(subPort.obj.callProxy(isInput?"output":"input", subPort.name).call<std::string>("domain"));
        auto actor = subPort.obj.callProxy("get:_actor");
        const auto subMode = actor.call<std::string>(isInput?"getOutputBufferMode":"getInputBufferMode", subPort.name, mainDomain);
        if (subMode != "ABDICATE") allOthersAbdicate = false;
    }

    //cant handle multiple domains
    if (subDomains.size() > 1) return false;

    assert(subDomains.size() == 1);
    const auto subDomain = *subDomains.begin();
    auto actor = mainPort.obj.callProxy("get:_actor");
    const auto mainMode = actor.call<std::string>(isInput?"getInputBufferMode":"getOutputBufferMode", mainPort.name, subDomain);

    //error always means we make a copy block
    if (mainMode == "ERROR") return false;

    //always good when we abdicate
    if (mainMode == "ABDICATE") return true;

    //otherwise the mode should be custom
    assert(mainMode == "CUSTOM");

    //cant handle custom with multiple upstream
    if (isInput and mainMode == "CUSTOM" and subPorts.size() > 1) return false;

    //if custom, the sub ports must abdicate
    if (mainMode == "CUSTOM" and not allOthersAbdicate) return false;

    return true;
}

static std::unordered_map<Port, Pothos::Proxy> domainInspection(
    const std::unordered_map<Port, std::vector<Port>> &ports,
    const bool isInput
)
{
    std::unordered_map<Port, Pothos::Proxy> bads;
    for (const auto &pair : ports)
    {
        if (isDomainCrossingAcceptable(pair.first, pair.second, isInput)) continue;
        auto registry = pair.first.obj.getEnvironment()->findProxy("Pothos/BlockRegistry");
        auto copier = registry.callProxy("/blocks/copier");
        copier.callVoid("setName", "DomainBridge");
        bads[pair.first] = copier;
    }

    return bads;
}

std::vector<Flow> Pothos::Topology::Impl::rectifyDomainFlows(const std::vector<Flow> &flatFlows)
{
    //map any given src or dst to its upstream/downstream ports
    std::unordered_map<Port, std::vector<Port>> srcs, dsts;
    for (const auto &flow : flatFlows)
    {
        for (const auto &subFlow : flatFlows)
        {
            if (subFlow.src == flow.src) srcs[flow.src].push_back(subFlow.dst);
            if (subFlow.dst == flow.dst) dsts[flow.dst].push_back(subFlow.src);
        }
    }

    //get a list of ports with domain problems
    auto badSrcsToCopier = domainInspection(srcs, false);
    auto badDstsToCopier = domainInspection(dsts, true);

    std::vector<Flow> domainSafeFlows;
    for (const auto &flow : flatFlows)
    {
        auto srcIt = badSrcsToCopier.find(flow.src);
        auto dstIt = badDstsToCopier.find(flow.dst);
        Pothos::Proxy copier;
        if (srcIt != badSrcsToCopier.end()) copier = srcIt->second;
        if (dstIt != badDstsToCopier.end()) copier = dstIt->second;
        if (copier)
        {
            //create the flows
            Flow srcFlow;
            srcFlow.src = flow.src;
            srcFlow.dst.obj = copier;
            srcFlow.dst.name = "0";

            Flow dstFlow;
            dstFlow.src.obj = copier;
            dstFlow.src.name = "0";
            dstFlow.dst = flow.dst;

            //add the network flows to the overall list
            domainSafeFlows.push_back(srcFlow);
            domainSafeFlows.push_back(dstFlow);
        }
        else
        {
            domainSafeFlows.push_back(flow);
        }
    }

    return domainSafeFlows;
}

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
 * Topology implementation
 **********************************************************************/
Pothos::Topology::Topology(void):
    _impl(new Impl())
{
    return;
}

static void topologyImplCleanup(Pothos::Topology *topology)
{
    if (topology->_impl and topology->_impl.unique())
    {
        topology->disconnectAll();
        topology->commit();
        assert(topology->_impl->activeFlatFlows.empty());
        assert(topology->_impl->flowToNetgressCache.empty());
    }
}

Pothos::Topology::~Topology(void)
{
    try
    {
        topologyImplCleanup(this);
    }
    catch (const Pothos::Exception &ex)
    {
        poco_error_f1(Poco::Logger::get("Pothos.Topology"), "Topology destructor threw: %s", ex.displayText());
    }
}

Pothos::Topology::Topology(const Topology &t):
    _impl(t._impl)
{
    return;
}

Pothos::Topology &Pothos::Topology::operator=(const Topology &t)
{
    topologyImplCleanup(this);
    _impl = t._impl;
    return *this;
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

void Pothos::Topology::commit(void)
{
    //1) flatten the topology
    auto flatFlows = squashFlows(_impl->flows);

    //2) create network iogress blocks when needed
    flatFlows = _impl->createNetworkFlows(flatFlows);

    //3) deal with domain crossing
    flatFlows = _impl->rectifyDomainFlows(flatFlows);

    //4) install buffer managers
    installBufferManagers(flatFlows);

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

    //result list is used to ack all de/activate messages
    std::vector<std::pair<std::string, Pothos::Proxy>> infoReceivers;

    //send activate to all new blocks not already in active flows
    for (auto block : getObjSetFromFlowList(newFlows, activeFlatFlows))
    {
        auto actor = block.callProxy("get:_actor");
        const auto msg = Poco::format("%s.sendActivateMessage()", block.call<std::string>("getName"));
        infoReceivers.push_back(std::make_pair(msg, actor.callProxy("sendActivateMessage")));
    }

    //update current flows
    _impl->activeFlatFlows = flatFlows;

    //send deactivate to all old blocks not in current active flows
    for (auto block : getObjSetFromFlowList(oldFlows, _impl->activeFlatFlows))
    {
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

void Pothos::Topology::_connect(
    const Object &src, const std::string &srcName,
    const Object &dst, const std::string &dstName)
{
    if (not checkObj(src)) throw Pothos::TopologyConnectError("Pothos::Topology::connect()",
        "source port of type " + src.toString());
    if (not checkObj(dst)) throw Pothos::TopologyConnectError("Pothos::Topology::connect()",
        "destination port of type " + dst.toString());

    Flow flow;
    flow.src.obj = getInternalObject(src, *this);
    flow.dst.obj = getInternalObject(dst, *this);
    flow.src.name = srcName;
    flow.dst.name = dstName;

    //perform auto-allocation, on a block this may or may not allocate, on a topology this throws
    try{getConnectable(src).callProxy("get:_actor").call<std::string>("autoAllocateOutput", srcName);}catch(const Exception &){}
    try{getConnectable(dst).callProxy("get:_actor").call<std::string>("autoAllocateInput", dstName);}catch(const Exception &){}

    const bool srcIsSelf = this->uid() == getConnectable(src).call<std::string>("uid");
    const bool dstIsSelf = this->uid() == getConnectable(dst).call<std::string>("uid");

    //validate that the ports exists before connection
    if (not srcIsSelf)
    {
        auto outs = getConnectable(src).call<std::vector<std::string>>("outputPortNames");
        if (std::find(outs.begin(), outs.end(), srcName) == outs.end())
            throw Pothos::TopologyConnectError("Pothos::Topology::connect()", src.toString() + " has no output port named " + srcName);
    }

    if (not dstIsSelf)
    {
        auto ins = getConnectable(dst).call<std::vector<std::string>>("inputPortNames");
        if (std::find(ins.begin(), ins.end(), dstName) == ins.end())
            throw Pothos::TopologyConnectError("Pothos::Topology::connect()", dst.toString() + " has no input port named " + dstName);
    }

    //store port info for connections to the hierachy
    if (srcIsSelf)
    {
        _impl->inputPortNames.push_back(srcName);
        for (const auto &info : getConnectable(dst).call<std::vector<PortInfo>>("inputPortInfo"))
        {
            if (info.name == dstName) _impl->inputPortInfo[info.name] = info;
            _impl->inputPortInfo[info.name].name = srcName;
        }
    }
    if (dstIsSelf)
    {
        _impl->outputPortNames.push_back(dstName);
        for (const auto &info : getConnectable(src).call<std::vector<PortInfo>>("outputPortInfo"))
        {
            if (info.name == srcName) _impl->outputPortInfo[info.name] = info;
            _impl->outputPortInfo[info.name].name = dstName;
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
    flow.src.obj = getInternalObject(src, *this);
    flow.dst.obj = getInternalObject(dst, *this);
    flow.src.name = srcName;
    flow.dst.name = dstName;

    //validate that the ports exists before disconnection
    auto outs = getConnectable(src).call<std::vector<std::string>>("outputPortNames");
    if (std::find(outs.begin(), outs.end(), srcName) == outs.end())
        throw Pothos::TopologyConnectError("Pothos::Topology::disconnect()", src.toString() + " has no output port named " + srcName);

    auto ins = getConnectable(dst).call<std::vector<std::string>>("inputPortNames");
    if (std::find(ins.begin(), ins.end(), dstName) == ins.end())
        throw Pothos::TopologyConnectError("Pothos::Topology::disconnect()", dst.toString() + " has no input port named " + dstName);

    //clear port info for disconnections from the hierachy
    if (this->uid() == getConnectable(src).call<std::string>("uid")) _impl->outputPortNames.erase(std::find(_impl->outputPortNames.begin(), _impl->outputPortNames.end(), srcName));
    if (this->uid() == getConnectable(dst).call<std::string>("uid")) _impl->inputPortNames.erase(std::find(_impl->inputPortNames.begin(), _impl->inputPortNames.end(), dstName));

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

Pothos::Object Pothos::Topology::opaqueCall(const std::string &name, const Object *inputArgs, const size_t numArgs)
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

static auto managedTopology = Pothos::ManagedClass()
    .registerConstructor<Pothos::Topology>()
    .registerBaseClass<Pothos::Topology, Pothos::Connectable>()
    .registerWildcardMethod(&Pothos::Topology::opaqueCall)
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
