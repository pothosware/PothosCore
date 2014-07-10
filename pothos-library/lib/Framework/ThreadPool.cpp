// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/ThreadPool.hpp>
#include <Poco/Environment.h>

Pothos::ThreadPoolArgs::ThreadPoolArgs(void):
    numThreads(Poco::Environment::processorCount()+1),
    priority(0.0),
    affinityMode("ALL"),
    yieldMode("CONDITION")
{
    return;
}

Pothos::ThreadPoolArgs::ThreadPoolArgs(const size_t numThreads):
    numThreads(numThreads),
    priority(0.0),
    affinityMode("ALL"),
    yieldMode("CONDITION")
{
    return;
}

Pothos::ThreadPool::ThreadPool(void)
{
    return;
}

Pothos::ThreadPool::ThreadPool(const ThreadPoolArgs &args)
{
    //TODO
}

Pothos::ThreadPool::operator bool(void) const
{
    return bool(_impl);
}
