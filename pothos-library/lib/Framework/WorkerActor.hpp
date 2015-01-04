// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Framework/BlockImpl.hpp>
#include <Pothos/Framework/Exception.hpp>
#include <Poco/Format.h>
#include <Poco/Logger.h>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <iostream>

template <typename T>
struct WorkerActorLock
{
    WorkerActorLock(T *actor):
        _actor(actor)
    {
        _actor->acquireContext();
    }
    ~WorkerActorLock(void)
    {
        _actor->releaseContext();
    }
    T *_actor;
};

/***********************************************************************
 * Actor definition
 **********************************************************************/
class Pothos::WorkerActor
{
public:
    WorkerActor(Block *block):
        block(block),
        activeState(false)
    {
        _condVarsWaiting = 0;
        _acquisitionCount = 0;
        _changeFlagged = false;
        _processDone = false;
        _acquirePrio = 0;
        _processThread = std::thread(std::bind(&Pothos::WorkerActor::processLoop, this));
    }

    void shutdown(void)
    {
        {
            WorkerActorLock<WorkerActor> lock(this);
            _processDone = true;
        }
        _processThread.join();
        this->outputs.clear();
        this->inputs.clear();
        this->updatePorts();
    }

    //////////////////////////////////// new threading impl ////////////////////////////////////

    size_t _condVarsWaiting, _acquisitionCount, _acquirePrio;
    bool _changeFlagged, _processDone;

    //std::atomic<size_t> _condVarsWaiting;
    //std::atomic<size_t> _acquisitionCount;
    //std::atomic<bool> _changeFlagged;
    std::mutex _acquisitionMutex;
    std::condition_variable _acquisitionCondVar;
    //std::atomic<bool> _processDone;
    std::thread _processThread;

    /*!
     * Acquire the call context for this actor.
     * If another thread has called acquire,
     * this call will block waiting for release.
     */
    void acquireContext(void)
    {
        std::unique_lock<std::mutex> lock(_acquisitionMutex);
        _acquirePrio++;
        while (_acquisitionCount != 0)
        {
            _acquisitionCondVar.wait(lock);
            //_acquisitionCondVar.wait_for(lock, std::chrono::milliseconds(1));
        }
        _acquisitionCount++;
    }

    /*!
     * Release the call context for this actor.
     * Every call to acquire must be matched.
     */
    void releaseEventContext(void)
    {
        {
            std::unique_lock<std::mutex> lock(_acquisitionMutex);
            _acquisitionCount--;
        }
        this->notifyWaiters();
    }

    void releaseContext(void)
    {
        {
            std::unique_lock<std::mutex> lock(_acquisitionMutex);
            _acquisitionCount--;
            _acquirePrio--;
            _changeFlagged = true;
        }
        this->notifyWaiters();
    }

    /*!
     * Indicate an event or resource change.
     */
    void flagChange(void)
    {
        {
            std::unique_lock<std::mutex> lock(_acquisitionMutex);
            _changeFlagged = true;
        }
        this->notifyWaiters();
    }

    //! flag change from inside context
    void flagChangeNoWake(void)
    {
        std::unique_lock<std::mutex> lock(_acquisitionMutex);
        _changeFlagged = true;
    }

    /*!
     * Wake up one thread waiting on the condition variable.
     */
    void notifyWaiters(void)
    {
        _acquisitionCondVar.notify_all();
    }

    /*!
     * Block in this call waiting for the change flag.
     */
    void acquireEventContext(void)
    {
        std::unique_lock<std::mutex> lock(_acquisitionMutex);
        while (not _changeFlagged or _acquisitionCount != 0 or _acquirePrio > 0)
        {
            _acquisitionCondVar.wait(lock);
            //_acquisitionCondVar.wait_for(lock, std::chrono::milliseconds(1));
        }
        _changeFlagged = false;
        _acquisitionCount++;
    }

    /*!
     * Perform the main processing action.
     */
    void processLoop(void)
    {
        while (not _processDone)
        {
            this->acquireEventContext();

            this->workTask();

            this->releaseEventContext();
        }
    }

    ///////////////////// WorkerActor storage ///////////////////////
    Block *block;
    bool activeState;
    WorkStats workStats;
    std::map<std::string, std::unique_ptr<InputPort>> inputs;
    std::map<std::string, std::unique_ptr<OutputPort>> outputs;

    ///////////////////// port setup methods ///////////////////////
    void allocateInput(const std::string &name, const DType &dtype, const std::string &domain);
    void allocateOutput(const std::string &name, const DType &dtype, const std::string &domain);
    void allocateSignal(const std::string &name);
    void allocateSlot(const std::string &name);
    template <typename PortsType, typename NamedPortsType, typename IndexedPortsType, typename PortNamesType>
    void allocatePort(PortsType &ports, NamedPortsType &namedPorts, IndexedPortsType &indexedPorts, PortNamesType &portNames, const std::string &name, const DType &dtype, const std::string &domain);

    void autoAllocateInput(const std::string &name);
    void autoAllocateOutput(const std::string &name);
    template <typename PortsType, typename NamedPortsType, typename IndexedPortsType, typename PortNamesType>
    void autoAllocatePort(PortsType &ports, NamedPortsType &namedPorts, IndexedPortsType &indexedPorts, PortNamesType &portNames, const std::string &name);

    //! call after making changes to ports
    void updatePorts(void);

    ///////////////////// topology helper methods ///////////////////////
    void setActiveStateOn(void);
    void setActiveStateOff(void);
    void subscribePort(
        const std::string &myPortName,
        Block *subscriberPortBlock,
        const std::string &subscriberPortName);
    void unsubscribePort(
        const std::string &myPortName,
        Block *subscriberPortBlock,
        const std::string &subscriberPortName);
    std::string getInputBufferMode(const std::string &name, const std::string &domain);
    std::string getOutputBufferMode(const std::string &name, const std::string &domain);
    BufferManager::Sptr getBufferManager(const std::string &name, const std::string &domain, const bool isInput);
    void setOutputBufferManager(const std::string &name, const BufferManager::Sptr &manager);

    ///////////////////// work helper methods ///////////////////////
    void workTask(void);
    bool preWorkTasks(void);
    void postWorkTasks(void);
};
