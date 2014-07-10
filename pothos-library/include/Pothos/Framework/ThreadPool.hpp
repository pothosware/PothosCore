//
// Framework/ThreadPool.hpp
//
// Support for configuring and managing threading in the framework.
//
// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <memory>
#include <string>
#include <vector>
#include <cstddef>

namespace Pothos {

/*!
 * Arguments used to configure a ThreadPool.
 */
class POTHOS_API ThreadPoolArgs
{
public:

    //! Create a default ThreadPoolArgs
    ThreadPoolArgs(void);

    //! Create a ThreadPoolArgs given a specific number of threads
    ThreadPoolArgs(const size_t numThreads);

    /*!
     * The number of threads to create in this pool.
     * The default value is 0, indicating automatic.
     */
    size_t numThreads;

    /*!
     * Scheduling priority for all threads in the pool.
     * The value can be in range -1.0 to 1.0.
     * A value of 0.0 is the default thread scheduling.
     * Positive values enable realtime scheduling mode.
     * Negative values enable sub-priority scheduling.
     *
     * The default is 0.0 (normal).
     */
    double priority;

    /*!
     * The affinity mode for this thread pool.
     * The affinityMode string can have the following values:
     *
     *  - "ALL" - affinitize to all avilable CPUs
     *  - "CPU" - affinity list specifies CPUs
     *  - "NUMA" - affinity list specifies NUMA nodes
     *
     * The default is "ALL".
     */
    std::string affinityMode;

    //! A list of CPUs or NUMA nodes (depends on mode setting)
    std::vector<size_t> affinity;

    /*!
     * The yieldMode specifies the internal threading mechanisms:
     * 
     *  - "CONDITION" - Threads wait on condition variables when no work is available.
     *  - "HYBRID" - Threads spin for a while, then yield to other threads, when no work is available.
     *  - "SPIN" - Threads busy-wait, without yielding, when no work is available.
     *
     * The default is "CONDITION".
     */
    std::string yieldMode;
};

/*!
 * A ThreadPool manages a group of threads that perform work in a Topology.
 * Not only can users configure the number of threads,
 * but there are a variety of other settings such as affinity,
 * real-time priority, and internal threading mechanisms.
 */
class POTHOS_API ThreadPool
{
public:

    //! Create a null ThreadPool
    ThreadPool(void);

    //! Create a ThreadPool from an opaque shared_ptr
    ThreadPool(const std::shared_ptr<void> &);

    /*!
     * Create a new ThreadPool from args.
     * \param args the configuration struct
     * \throws ThreadPoolError on bad values
     */
    ThreadPool(const ThreadPoolArgs &args);

    /*!
     * Is this thread pool valid/non-empty?
     * \return true when the thread poll is non-null
     */
    pothos_explicit operator bool(void) const;

    /*!
     * Get access to the underlying container for the thread pool.
     * \return an opaque shared_ptr to the internal object
     */
    const std::shared_ptr<void> &getContainer(void) const;

private:
    std::shared_ptr<void> _impl;
};

} //namespace Pothos
