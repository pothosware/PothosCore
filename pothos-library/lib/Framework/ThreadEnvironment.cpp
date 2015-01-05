// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/ThreadEnvironment.hpp"
#include <iostream>

ThreadEnvironment::ThreadEnvironment(const size_t numThreads):
    _numThreads(numThreads)
{
    return;
}

ThreadEnvironment::~ThreadEnvironment(void)
{
    while (not _actorToTask.empty())
    {
        this->unregisterActor(_actorToTask.begin()->first);
    }
}

void ThreadEnvironment::registerActor(void *actor, ThreadEnvironment::Task task)
{
    std::unique_lock<std::mutex> lock(_registrationMutex);

    {
        std::unique_lock<std::mutex> lock0(_actorUpdateMutex);
        _actorToTask[actor] = task;
    }

    _configurationSignature++;

    if (_numThreads == 0)
    {
        _actorToThread[actor] = std::thread(std::bind(&ThreadEnvironment::singleProcessLoop, this, actor));
    }
    else
    {
        if (_threadPool.size() < _numThreads)
        {
            size_t index = _threadPool.size();
            _threadPool.push_back(std::thread(std::bind(&ThreadEnvironment::poolProcessLoop, this, index)));
        }
    }
}

void ThreadEnvironment::unregisterActor(void *actor)
{
    std::unique_lock<std::mutex> lock(_registrationMutex);

    {
        std::unique_lock<std::mutex> lock0(_actorUpdateMutex);
        _actorToTask.erase(actor);
    }

    _configurationSignature++;

    if (_numThreads == 0)
    {
        _actorToThread[actor].join();
        _actorToThread.erase(actor);
    }
    else
    {
        if (_threadPool.size() > _actorToTask.size())
        {
            _threadPool.back().join();
            _threadPool.resize(_actorToTask.size());
        }
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
            std::unique_lock<std::mutex> lock(_actorUpdateMutex);
            localTasks = _actorToTask;
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

void ThreadEnvironment::singleProcessLoop(void *actor)
{
    size_t localSignature = 0;
    std::map<void *, Task> localTasks;
    auto it = localTasks.end();

    while (true)
    {
        //check for a configuration change and update the local state
        if (_configurationSignature != localSignature)
        {
            std::unique_lock<std::mutex> lock(_actorUpdateMutex);
            localTasks = _actorToTask;
            it = localTasks.find(actor);
            localSignature = _configurationSignature;
        }

        //actor mode, actor not in tasks
        if (it == localTasks.end()) return;

        //perform the task
        it->second(0);
    }
}
