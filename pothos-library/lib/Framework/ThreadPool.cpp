// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/ThreadPool.hpp>
#include <Pothos/Framework/Exception.hpp>
#include <Poco/Environment.h>
#include <Theron/Actor.h>
#include <Theron/Framework.h>

Pothos::ThreadPoolArgs::ThreadPoolArgs(void):
    numThreads(0),
    priority(0.0)
{
    return;
}

Pothos::ThreadPoolArgs::ThreadPoolArgs(const size_t numThreads):
    numThreads(numThreads),
    priority(0.0)
{
    return;
}

Pothos::ThreadPool::ThreadPool(void)
{
    return;
}

Pothos::ThreadPool::ThreadPool(const std::shared_ptr<void> &impl):
    _impl(impl)
{
    return;
}

static uint32_t toMask(const Pothos::ThreadPoolArgs &args)
{
    uint32_t mask = 0;
    for (auto i : args.affinity) mask |= (1 << i);
    return mask;
}

Pothos::ThreadPool::ThreadPool(const ThreadPoolArgs &args)
{
    //create params for the number of threads
    Theron::Framework::Parameters params(args.numThreads, ~0);
    if (args.numThreads == 0) params.mThreadCount = Poco::Environment::processorCount()+1;

    //setup affinity masks
    if (args.affinityMode.empty()){/* no changes to default masks */}
    else if (args.affinityMode == "ALL"){/* no changes to default masks */}
    else if (args.affinityMode == "CPU") params.mProcessorMask = toMask(args);
    else if (args.affinityMode == "NUMA") params.mNodeMask = toMask(args);
    else throw ThreadPoolError("Pothos::ThreadPool()", "unknown affinityMode " + args.affinityMode);

    //setup yield strategy
    if (args.yieldMode.empty()) params.mYieldStrategy = Theron::YIELD_STRATEGY_CONDITION;
    else if (args.yieldMode == "CONDITION") params.mYieldStrategy = Theron::YIELD_STRATEGY_CONDITION;
    else if (args.yieldMode == "HYBRID") params.mYieldStrategy = Theron::YIELD_STRATEGY_HYBRID;
    else if (args.yieldMode == "SPIN") params.mYieldStrategy = Theron::YIELD_STRATEGY_SPIN;
    else throw ThreadPoolError("Pothos::ThreadPool()", "unknown yieldMode " + args.yieldMode);

    //thread priority
    if (args.priority > +1.0 or args.priority < -1.0) throw ThreadPoolError("Pothos::ThreadPool()", "priority out of range " + std::to_string(args.priority));
    params.mThreadPriority = float(args.priority);

    std::shared_ptr<Theron::Framework> framework(new Theron::Framework(params));
    _impl = framework;
}

Pothos::ThreadPool::operator bool(void) const
{
    return bool(_impl);
}

const std::shared_ptr<void> &Pothos::ThreadPool::getContainer(void) const
{
    return _impl;
}

bool Pothos::operator==(const ThreadPool &lhs, const ThreadPool &rhs)
{
    return lhs.getContainer() == rhs.getContainer();
}

#include <Pothos/Managed.hpp>

static auto managedThreadPoolArgs = Pothos::ManagedClass()
    .registerConstructor<Pothos::ThreadPoolArgs>()
    .registerConstructor<Pothos::ThreadPoolArgs, const size_t>()
    .registerField(POTHOS_FCN_TUPLE(Pothos::ThreadPoolArgs, numThreads))
    .registerField(POTHOS_FCN_TUPLE(Pothos::ThreadPoolArgs, priority))
    .registerField(POTHOS_FCN_TUPLE(Pothos::ThreadPoolArgs, affinityMode))
    .registerField(POTHOS_FCN_TUPLE(Pothos::ThreadPoolArgs, affinity))
    .registerField(POTHOS_FCN_TUPLE(Pothos::ThreadPoolArgs, yieldMode))
    .commit("Pothos/ThreadPoolArgs");

static auto managedThreadPool = Pothos::ManagedClass()
    .registerConstructor<Pothos::ThreadPool>()
    .registerConstructor<Pothos::ThreadPool, const std::shared_ptr<void> &>()
    .registerConstructor<Pothos::ThreadPool, const Pothos::ThreadPoolArgs &>()
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::ThreadPool, getContainer))
    .registerStaticMethod<const Pothos::ThreadPool &, const Pothos::ThreadPool &>("equal", Pothos::operator==)
    .commit("Pothos/ThreadPool");

#include <Pothos/Object/Serialize.hpp>

namespace Pothos { namespace serialization {
template <class Archive>
void serialize(Archive &ar, Pothos::ThreadPoolArgs &t, const unsigned int)
{
    ar & t.numThreads;
    ar & t.priority;
    ar & t.affinityMode;
    ar & t.affinity;
    ar & t.yieldMode;
}
}}

POTHOS_OBJECT_SERIALIZE(Pothos::ThreadPoolArgs)
