// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/ThreadEnvironment.hpp"
#include <cassert>

ThreadEnvironment::ThreadEnvironment(const size_t numThreads):
    _numThreads(numThreads)
{
    return;
}

ThreadEnvironment::~ThreadEnvironment(void)
{
    //tear-down all tasks if not done by caller
    while (not _handleToTask.empty())
    {
        this->unregisterTask(_handleToTask.begin()->first);
    }
}

void ThreadEnvironment::registerTask(void *handle, ThreadEnvironment::Task task)
{
    std::unique_lock<std::mutex> lock(_registrationMutex);

    //register the new task and bump the signature to notify threads
    {
        std::unique_lock<std::mutex> lock0(_handleUpdateMutex);
        _handleToTask[handle] = task;
    }
    _configurationSignature++;

    //single task mode: spawn a new thread for this task
    if (_numThreads == 0)
    {
        _handleToThread[handle] = std::thread(std::bind(&ThreadEnvironment::singleProcessLoop, this, handle));
    }

    //pool mode: start a thread if the pool size is too small
    else
    {
        if (_threadPool.size() < _numThreads)
        {
            size_t index = _threadPool.size();
            _threadPool.push_back(std::thread(std::bind(&ThreadEnvironment::poolProcessLoop, this, index)));
        }
        assert(_threadPool.size() <= _numThreads);
    }
}

void ThreadEnvironment::unregisterTask(void *handle)
{
    std::unique_lock<std::mutex> lock(_registrationMutex);

    //unregister the new task and bump the signature to notify threads
    {
        std::unique_lock<std::mutex> lock0(_handleUpdateMutex);
        _handleToTask.erase(handle);
    }
    _configurationSignature++;

    //single task mode: stop the explicit task for this handle
    if (_numThreads == 0)
    {
        _handleToThread[handle].join();
        _handleToThread.erase(handle);
    }

    //pool mode: stop a thread if the pool size is too large
    else
    {
        if (_threadPool.size() > _handleToTask.size())
        {
            _threadPool.back().join();
            _threadPool.resize(_handleToTask.size());
        }
        assert(_threadPool.size() <= _numThreads);
    }
}

void ThreadEnvironment::poolProcessLoop(size_t index)
{
    size_t localSignature = 0;
    std::map<void *, Task> localTasks;
    auto it = localTasks.end();

    while (true)
    {
        //check for a configuration change and update the local state
        if (_configurationSignature != localSignature)
        {
            std::unique_lock<std::mutex> lock(_handleUpdateMutex);
            localTasks = _handleToTask;
            it = localTasks.end();
            localSignature = _configurationSignature;
        }

        //pool mode, index out of range
        if (index >= localTasks.size()) return;

        //perform a task and increment
        if (it == localTasks.end()) it = localTasks.begin();
        it->second(0);
        it++;
    }
}

void ThreadEnvironment::singleProcessLoop(void *handle)
{
    size_t localSignature = 0;
    std::map<void *, Task> localTasks;
    auto it = localTasks.end();

    while (true)
    {
        //check for a configuration change and update the local state
        if (_configurationSignature != localSignature)
        {
            std::unique_lock<std::mutex> lock(_handleUpdateMutex);
            localTasks = _handleToTask;
            it = localTasks.find(handle);
            localSignature = _configurationSignature;
        }

        //handle mode, handle not in tasks
        if (it == localTasks.end()) return;

        //perform the task
        it->second(0);
    }
}
