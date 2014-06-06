// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/TopologyComprehension.hpp"
#include "Framework/WorkerActor.hpp"
#include <Pothos/Framework/Block.hpp>
#include <Pothos/Framework/Exception.hpp>
#include <Pothos/Object.hpp>
#include <Pothos/Proxy.hpp>
#include <Poco/Environment.h>
#include <Poco/Format.h>
#include <Poco/Timestamp.h>
#include <Poco/Timespan.h>
#include <Poco/Thread.h> //sleep
#include <algorithm>
#include <vector>
#include <cassert>
#include <set>

static Pothos::Proxy getProxy(const Pothos::Object &o)
{
    if (o.type() == typeid(Pothos::Proxy)) return o.extract<Pothos::Proxy>();
    return Pothos::ProxyEnvironment::make("managed")->convertObjectToProxy(o);
}

/***********************************************************************
 * Extract Actor for arbitrary object
 **********************************************************************/
static Pothos::Proxy getWorkerActorInterface(const Pothos::Proxy &block)
{
    auto actor = block.callProxy("actor");
    //TODO FIXME unwrap a proxy in a proxy
    //This happens when PythonBlock actor call returns a proxy to the actor
    //There must be a better way to deal with metaproxy stuff.
    if (actor.getClassName() == "PothosProxy") actor = actor.convert<Pothos::Proxy>();
    assert(actor.getEnvironment()->getName() == "managed");
    auto cls = actor.getEnvironment()->findProxy("Pothos/WorkerActorInterface");
    return cls.callProxy("new", actor);
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

    return flatFlows;
}

/***********************************************************************
 * helpers to create network iogress flows
 **********************************************************************/
std::vector<Flow> Pothos::Topology::Impl::createNetworkFlows()
{
    //first flatten the topology
    const auto flatFlows = squashFlows(this->flows);

    //then create network iogress blocks when needed
    std::vector<Flow> networkAwareFlows;
    for (const auto &flow : flatFlows)
    {
        auto srcActorIface = getWorkerActorInterface(flow.src.obj);
        auto dstActorIface = getWorkerActorInterface(flow.dst.obj);

        //get unique process ids
        const auto srcUpid = srcActorIface.call<std::string>("upid");
        const auto dstUpid = dstActorIface.call<std::string>("upid");

        //same process, keep this flow as-is
        if (srcUpid == dstUpid)
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
            //create source and sink blocks
            auto srcEnvReg = srcActorIface.getEnvironment()->findProxy("Pothos/BlockRegistry");
            auto dstEnvReg = dstActorIface.getEnvironment()->findProxy("Pothos/BlockRegistry");

            auto netSink = srcEnvReg.callProxy("/blocks/network/network_sink", "udt://"+Poco::Environment::nodeName(), "BIND", srcActorIface.callProxy("getPortDType", false, flow.src.name));
            auto connectPort = netSink.call<std::string>("getActualPort");
            auto connectUri = Poco::format("udt://%s:%s", Poco::Environment::nodeName(), connectPort);
            auto netSource = dstEnvReg.callProxy("/blocks/network/network_source", connectUri, "CONNECT", dstActorIface.callProxy("getPortDType", true, flow.dst.name));

            //create the flows
            Flow srcFlow;
            srcFlow.src = flow.src;
            srcFlow.dst.obj = netSink;
            srcFlow.dst.name = "0";

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
 * Helpers to implement port subscription
 **********************************************************************/
static void updateFlows(const std::vector<Flow> &flows, const std::string &action)
{
    //result list is used to ack all subscribe messages
    std::vector<Pothos::Proxy> resultActorIfaces;

    //add new data acceptors
    for (const auto &flow : flows)
    {
        auto srcActorIface = getWorkerActorInterface(flow.src.obj);
        auto dstActorIface = getWorkerActorInterface(flow.dst.obj);

        if (action == "SUBINPUT" or action == "UNSUBINPUT")
        {
            srcActorIface.call("sendPortSubscriberMessage", action, flow.src.name, flow.dst.name, dstActorIface.callProxy("getAddress"));
            resultActorIfaces.push_back(srcActorIface);
        }
        if (action == "SUBOUTPUT" or action == "UNSUBOUTPUT")
        {
            dstActorIface.call("sendPortSubscriberMessage", action, flow.dst.name, flow.src.name, srcActorIface.callProxy("getAddress"));
            resultActorIfaces.push_back(dstActorIface);
        }
    }

    //check all subscribe message results
    for (auto actorIface : resultActorIfaces)
    {
        const auto &msg = actorIface.call<std::string>("waitStringResult");
        if (msg.empty()) continue;
        throw Pothos::TopologyConnectError("Pothos::Exectutor::commit()", msg);
    }
}

static std::map<std::string, Pothos::Proxy> getActorInterfacesInFlowList(const std::vector<Flow> &flows, const std::vector<Flow> &excludes = std::vector<Flow>())
{
    std::map<std::string, Pothos::Proxy> interfaces;
    for (const auto &flow : flows)
    {
        interfaces[flow.src.obj.call<std::string>("uid")] = getWorkerActorInterface(flow.src.obj);
        interfaces[flow.dst.obj.call<std::string>("uid")] = getWorkerActorInterface(flow.dst.obj);
    }
    for (const auto &flow : excludes)
    {
        interfaces.erase(flow.src.obj.call<std::string>("uid"));
        interfaces.erase(flow.dst.obj.call<std::string>("uid"));
    }
    return interfaces;
}

/***********************************************************************
 * Topology implementation
 **********************************************************************/
Pothos::Topology::Topology(void):
    _impl(new Impl())
{
    return;
}

Pothos::Topology::~Topology(void)
{
    this->disconnectAll();
    this->commit();
    assert(_impl->activeFlatFlows.empty());
    assert(_impl->flowToNetgressCache.empty());
}

void Pothos::Topology::commit(void)
{
    const auto flatFlows = _impl->createNetworkFlows();
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

    //add new data acceptors
    updateFlows(newFlows, "SUBINPUT");

    //add new data providers
    updateFlows(newFlows, "SUBOUTPUT");

    //remove old data providers
    updateFlows(oldFlows, "UNSUBOUTPUT");

    //remove old data acceptors
    updateFlows(oldFlows, "UNSUBINPUT");

    //result list is used to ack all de/activate messages
    std::vector<Pothos::Proxy> resultActorIfaces;

    //send activate to all new blocks not already in active flows
    for (auto pair : getActorInterfacesInFlowList(newFlows, activeFlatFlows))
    {
        pair.second.call("sendActivateMessage");
        resultActorIfaces.push_back(pair.second);
    }

    //update current flows
    _impl->activeFlatFlows = flatFlows;

    //send deactivate to all old blocks not in current active flows
    for (auto pair : getActorInterfacesInFlowList(oldFlows, _impl->activeFlatFlows))
    {
        pair.second.call("sendDeactivateMessage");
        resultActorIfaces.push_back(pair.second);
    }

    //check all de/activate message results
    for (auto actorIface : resultActorIfaces)
    {
        const auto &msg = actorIface.call<std::string>("waitStringResult");
        if (msg.empty()) continue;
        throw Pothos::TopologyConnectError("Pothos::Exectutor::commit()", msg);
    }

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
    /*
    if (getUid(src).empty()) throw Pothos::TopologyConnectError("Pothos::Topology::connect()",
        "source port of type " + src.toString());
    if (getUid(dst).empty()) throw Pothos::TopologyConnectError("Pothos::Topology::connect()",
        "destination port of type " + dst.toString());
        */

    Flow flow;
    std::cout << __LINE__ << std::endl;
    flow.src.obj = getInternalObject(src, *this);
    std::cout << __LINE__ << std::endl;
    flow.dst.obj = getInternalObject(dst, *this);
    std::cout << __LINE__ << std::endl;
    flow.src.name = srcName;
    flow.dst.name = dstName;

    const auto it = std::find(_impl->flows.begin(), _impl->flows.end(), flow);
    if (it != _impl->flows.end()) throw Pothos::TopologyConnectError("Pothos::Topology::connect()",
        "this flow already exists in the topology");

    _impl->flows.push_back(flow);
}

void Pothos::Topology::_disconnect(
    const Object &src, const std::string &srcName,
    const Object &dst, const std::string &dstName)
{
    /*
    if (getUid(src).empty()) throw Pothos::TopologyConnectError("Pothos::Topology::disconnect()",
        "source port of type " + src.toString());
    if (getUid(dst).empty()) throw Pothos::TopologyConnectError("Pothos::Topology::disconnect()",
        "destination port of type " + dst.toString());
    */

    Flow flow;
    flow.src.obj = getInternalObject(src, *this);
    flow.dst.obj = getInternalObject(dst, *this);
    flow.src.name = srcName;
    flow.dst.name = dstName;

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
            flow.src.obj.call("disconnectAll");
        }
        catch (const Pothos::Exception &){}

        if (flow.dst.obj) try
        {
            flow.dst.obj.call("disconnectAll");
        }
        catch (const Pothos::Exception &){}
    }

    //clear our own local flows
    _impl->flows.clear();
}

bool Pothos::Topology::waitInactive(const double idleDuration, const double timeout)
{
    //how long to sleep between idle checks?
    const double pollSleepTime = idleDuration/3;

    //get a list of actor interfaces to poll for idle time
    const auto interfaces = getActorInterfacesInFlowList(_impl->activeFlatFlows);

    //loop until exit time
    const Poco::Timestamp exitTime = Poco::Timestamp() + Poco::Timespan(Poco::Timespan::TimeDiff(timeout*1e6));
    do
    {
        //check each worker for idle time from the stats
        for (auto pair : interfaces)
        {
            const auto stats = pair.second.call<WorkerStats>("getWorkerStats");
            const auto consumptionIdle = stats.ticksStatsQuery - stats.ticksLastConsumed;
            const auto productionIdle = stats.ticksStatsQuery - stats.ticksLastProduced;
            const auto workerIdleDuration = std::min(consumptionIdle, productionIdle);
            if (workerIdleDuration < idleDuration*stats.tickRate) goto pollSleep;
        }

        //all workers reached the max idle time specified
        return true;

        //idle time not reached on any workers, therefore sleep
        pollSleep: Poco::Thread::sleep(long(pollSleepTime*1e3));
    }
    while (Poco::Timestamp() < exitTime);

    return false; //timeout
}

#include <Pothos/Managed.hpp>

static const std::string &getUidFromTopology(const Pothos::Topology &t)
{
    return t.uid();
}

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
    .registerMethod("uid", &getUidFromTopology)
    .registerMethod("getFlows", &getFlowsFromTopology)
    .registerMethod("resolvePorts", &resolvePortsFromTopology)
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Topology, commit))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Topology, disconnectAll))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Topology, waitInactive))
    //and bind defaults into waitInactive for optional trailing arguments
    .registerMethod("waitInactive", Pothos::Callable(&Pothos::Topology::waitInactive).bind(1.0, 2))
    .registerMethod("waitInactive", Pothos::Callable(&Pothos::Topology::waitInactive).bind(1.0, 2).bind(0.1, 1))
    .registerMethod("connect", &Pothos::Topology::_connect)
    .registerMethod("disconnect", &Pothos::Topology::_disconnect)
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
