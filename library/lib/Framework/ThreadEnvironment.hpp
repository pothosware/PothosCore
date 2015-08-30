// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Framework/ThreadPool.hpp>
#include <memory>
#include <mutex>
#include <functional>
#include <atomic>
#include <thread>
#include <vector>
#include <map>

/*!
 * Storage container for a worker task and an atomic flag.
 * The flag is used for exclusive access in pool mode.
 */
struct TaskData
{
    typedef std::function<bool(bool)> Task;
    typedef std::function<void(void)> Wake;

    TaskData(const Task task, const Wake wake):
        task(task),
        wake(wake)
    {
        flag.clear(std::memory_order_release);
    }

    Task task;
    Wake wake;
    std::atomic_flag flag;
};

/*!
 * ThreadEnvironment is the implementation details for ThreadPool.
 * It manages groups of threads, configuration, task dispatching.
 */
class ThreadEnvironment
{
public:
    ThreadEnvironment(const Pothos::ThreadPoolArgs &args);

    ~ThreadEnvironment(void);

    /*!
     * Register a task with this thread environment.
     * \param handle a unique handle representing the caller
     * \param task a function pointer to the handle worker task
     * \param wake a function pointer to wake a worker task
     */
    void registerTask(void *handle, TaskData::Task task, TaskData::Wake wake);

    /*!
     * Unregister the task from the thread environment.
     * \param handle the unique handle used to register
     */
    void unregisterTask(void *handle);

    //! Query the thread pool construction args
    const Pothos::ThreadPoolArgs &getArgs(void) const
    {
        return _args;
    }

    /*!
     * Is waiting allowed within an task?
     */
    bool isWaitingEnabled(void) const
    {
        return _waitModeEnabled;
    }

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

    //! Set thread prio - return error message
    static std::string setPriority(const double prio);

    //! Set CPU affinity - return error message
    static std::string setCPUAffinity(const std::vector<size_t> &affinity);

    //! Set NUMA affinity - return error message
    static std::string setNodeAffinity(const std::vector<size_t> &affinity);

    //the thread pool configuration arguments
    Pothos::ThreadPoolArgs _args;

    //whether or not waiting is allowed based on args
    bool _waitModeEnabled;

    //map of handle handles to tasks
    std::map<void *, std::shared_ptr<TaskData>> _handleToTask;

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
