// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <mutex>
#include <functional>
#include <atomic>
#include <thread>
#include <vector>
#include <map>

class ThreadEnvironment
{
public:
    ThreadEnvironment(const size_t numThreads);

    ~ThreadEnvironment(void);

    typedef std::function<void(int)> Task;

    /*!
     * Register an actor with this thread environment.
     * \param actor a unique handle representing the actor pointer
     * \param task a function pointer to the actor worker task
     */
    void registerActor(void *actor, Task task);

    /*!
     * Unregister the actor from the thread environment.
     * \param actor a unique handle representing the actor pointer
     */
    void unregisterActor(void *actor);

private:
    /*!
     * Process loop used in thread pool mode:
     * The index specifies the thread index.
     * If the index is out of range given
     * the number of actors, the thread exits.
     */
    void poolProcessLoop(size_t index);

    /*!
     * Process loop used in thread per actor mode.
     * If the actor is removed, the thread exists.
     */
    void singleProcessLoop(void *actor);

    //the maximum number of threads or 0 for thread per actor mode
    size_t _numThreads;

    //map of actor handles to tasks
    std::map<void *, Task> _actorToTask;

    //configuration signature (changed when actor list changed)
    std::atomic<size_t> _configurationSignature;

    //mutex for protecting actor registration
    std::mutex _registrationMutex;

    //mutex for protecting actor updates to the thread pool
    std::mutex _actorUpdateMutex;

    //actor to thread map (used in actor per thread mode)
    std::map<void *, std::thread> _actorToThread;

    //per-thread process loop done flags (used in thread pool mode)
    std::vector<std::thread> _threadPool;
};
