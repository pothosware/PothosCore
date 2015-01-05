// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Framework/ThreadPool.hpp>
#include <mutex>
#include <functional>
#include <atomic>
#include <thread>
#include <vector>
#include <map>

class ThreadEnvironment
{
public:
    ThreadEnvironment(const Pothos::ThreadPoolArgs &args);

    ~ThreadEnvironment(void);

    typedef std::function<void(int)> Task;

    /*!
     * Register a task with this thread environment.
     * \param handle a unique handle representing the caller
     * \param task a function pointer to the handle worker task
     */
    void registerTask(void *handle, Task task);

    /*!
     * Unregister the task from the thread environment.
     * \param handle the unique handle used to register
     */
    void unregisterTask(void *handle);

private:
    /*!
     * Process loop used in thread pool mode:
     * The index specifies the thread index.
     * If the index is out of range given
     * the number of handles, the thread exits.
     */
    void poolProcessLoop(size_t index);

    /*!
     * Process loop used in thread per task mode.
     * If the handle is removed, the thread exists.
     */
    void singleProcessLoop(void *handle);

    /*!
     * Apply priority and affinity to the caller.
     * This call uses the thread config in _args.
     */
    void applyThreadConfig(void);

    //the maximum number of threads or 0 for thread per handle mode
    Pothos::ThreadPoolArgs _args;

    //map of handle handles to tasks
    std::map<void *, Task> _handleToTask;

    //configuration signature (changed when handle list changed)
    std::atomic<size_t> _configurationSignature;

    //mutex for protecting handle registration
    std::mutex _registrationMutex;

    //mutex for protecting handle updates to the thread pool
    std::mutex _handleUpdateMutex;

    //handle to thread map (used in handle per thread mode)
    std::map<void *, std::thread> _handleToThread;

    //per-thread process loop done flags (used in thread pool mode)
    std::vector<std::thread> _threadPool;
};
