// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/Topology.hpp>
#include "Framework/PortsAndFlows.hpp"
#include "Framework/WorkerStats.hpp"
#include <Pothos/Framework/Block.hpp>
#include <Pothos/Framework/Exception.hpp>
#include <Pothos/Object.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/System/Info.hpp>
#include <Poco/Environment.h>
#include <Poco/Format.h>
#include <Poco/Timestamp.h>
#include <Poco/Timespan.h>
#include <Poco/Thread.h> //sleep
#include <Poco/NumberParser.h>
#include <algorithm>
#include <unordered_map>
#include <vector>
#include <cassert>
#include <set>
#include <sstream>
#include <cctype>

/***********************************************************************
 * implementation guts
 **********************************************************************/
struct Pothos::Topology::Impl
{
    std::string name;
    std::vector<Flow> flows;
    std::vector<Flow> activeFlatFlows;
    std::unordered_map<Flow, std::pair<Flow, Flow>> flowToNetgressCache;
    std::vector<Flow> createNetworkFlows(void);
};

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
 * get a unique process identifier for an environment
 **********************************************************************/
static std::string getUpid(const Pothos::ProxyEnvironment::Sptr &env)
{
    assert(env->getName() == "managed");
    auto info = env->findProxy("Pothos/System/NodeInfo").call<Pothos::System::NodeInfo>("get");
    return info.nodeName + "/" + info.nodeId + "/" + info.pid;
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
std::vector<Flow> Pothos::Topology::Impl::createNetworkFlows()
{
    //first flatten the topology
    const auto flatFlows = squashFlows(this->flows);

    //then create network iogress blocks when needed
    std::vector<Flow> networkAwareFlows;
    for (const auto &flow : flatFlows)
    {
        //same process, keep this flow as-is
        if (getUpid(flow.src.obj.getEnvironment()) == getUpid(flow.dst.obj.getEnvironment()))
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
            auto srcEnvReg = flow.src.obj.getEnvironment()->findProxy("Pothos/BlockRegistry");
            auto dstEnvReg = flow.dst.obj.getEnvironment()->findProxy("Pothos/BlockRegistry");

            //determine port types
            auto srcDType = flow.src.obj.callProxy("output", flow.src.name).callProxy("dtype");
            auto dstDType = flow.dst.obj.callProxy("input", flow.dst.name).callProxy("dtype");

            auto netSink = srcEnvReg.callProxy("/blocks/network/network_sink", "udt://"+Poco::Environment::nodeName(), "BIND", srcDType);
            netSink.callVoid("setName", "NetOut");
            auto connectPort = netSink.call<std::string>("getActualPort");
            auto connectUri = Poco::format("udt://%s:%s", Poco::Environment::nodeName(), connectPort);
            auto netSource = dstEnvReg.callProxy("/blocks/network/network_source", connectUri, "CONNECT", dstDType);
            netSource.callVoid("setName", "NetIn");

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
    const bool isInputAction = action.find("INPUT") != std::string::npos;

    //result list is used to ack all subscribe messages
    std::vector<std::pair<std::string, Pothos::Proxy>> infoReceivers;

    //add new data acceptors
    for (const auto &flow : flows)
    {
        const auto &pri = isInputAction?flow.src:flow.dst;
        const auto &sec = isInputAction?flow.dst:flow.src;

        auto actor = pri.obj.callProxy("get:_actor");
        auto result = actor.callProxy("sendPortSubscriberMessage", action, pri.name, sec.obj.callProxy("getCPointer"), sec.name);
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

static std::vector<Pothos::Proxy> getObjSetFromFlowList(const std::vector<Flow> &flows, const std::vector<Flow> &excludes = std::vector<Flow>())
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
 * Topology implementation
 **********************************************************************/
Pothos::Topology::Topology(void):
    _impl(new Impl())
{
    this->setName("Topology");
}

Pothos::Topology::~Topology(void)
{
    this->disconnectAll();
    this->commit();
    assert(_impl->activeFlatFlows.empty());
    assert(_impl->flowToNetgressCache.empty());
}

void Pothos::Topology::setName(const std::string &name)
{
    _impl->name = name;
}

const std::string &Pothos::Topology::getName(void) const
{
    return _impl->name;
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
    const double pollSleepTime = idleDuration/3;

    //get a list of blocks to poll for idle time
    const auto blocks = getObjSetFromFlowList(_impl->activeFlatFlows);

    //loop until exit time
    const Poco::Timestamp exitTime = Poco::Timestamp() + Poco::Timespan(Poco::Timespan::TimeDiff(timeout*1e6));
    do
    {
        //check each worker for idle time from the stats
        for (auto block : blocks)
        {
            const auto stats = block.call<WorkerStats>("getWorkerStats");
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

static std::string getDotEscapedString(const Pothos::Proxy &elem)
{
    std::string out;
    for (const char ch : elem.call<std::string>("getName"))
    {
        if (std::isalnum(ch) or ch == '_') out.push_back(ch);
        else out.push_back('_');
    }
    return out;
}

std::string Pothos::Topology::toDotMarkup(const bool flat)
{
    std::ostringstream os;
    auto flows = (flat)? _impl->activeFlatFlows : _impl->flows;
    auto blocks = getObjSetFromFlowList(flows);

    os << "digraph flat_flows {" << std::endl;
    os << "    rankdir=LR;" << std::endl;
    os << "    node [shape=record, fontsize=10];" << std::endl;

    for (const auto &block : blocks)
    {
        unsigned value = 0;
        os << "    ";
        os << block.callProxy("uid").hashCode();
        os << " [shape=record, label=\"{ ";
        std::string inPortsStr;
        for (const auto &name : block.call<std::vector<std::string>>("inputPortNames"))
        {
            if (not inPortsStr.empty()) inPortsStr += " | ";
            inPortsStr += "<__in__"+name+"> ";
            if (Poco::NumberParser::tryParseUnsigned(name, value)) inPortsStr += "in";
            inPortsStr += name;
        }
        std::string outPortsStr;
        for (const auto &name : block.call<std::vector<std::string>>("outputPortNames"))
        {
            if (not outPortsStr.empty()) outPortsStr += " | ";
            outPortsStr += "<__out__"+name+"> ";
            if (Poco::NumberParser::tryParseUnsigned(name, value)) outPortsStr += "out";
            outPortsStr += name;
        }
        if (not inPortsStr.empty()) os << " { " << inPortsStr << " } | ";
        os << " " << getDotEscapedString(block) << " ";
        if (not outPortsStr.empty()) os << " | { " << outPortsStr << " } ";
        os << " }\" style=filled, fillcolor=\"azure\"];" << std::endl;
    }

    for (const auto &flow : flows)
    {
        os << "    ";
        os << flow.src.obj.callProxy("uid").hashCode() << ":__out__" << flow.src.name;
        os << " -> ";
        os << flow.dst.obj.callProxy("uid").hashCode() << ":__in__" << flow.dst.name;
        os << ";" << std::endl;
    }

    os << "}" << std::endl;
    return os.str();
}

#include <Pothos/Managed.hpp>

//FIXME see issue #37
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
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Topology, setName))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Topology, getName))
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
