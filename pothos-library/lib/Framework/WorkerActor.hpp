// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Framework/BlockImpl.hpp>
#include <Pothos/Framework/Exception.hpp>
#include <Pothos/Object/Containers.hpp>
#include <Theron/Actor.h>
#include <Theron/Framework.h>
#include <Theron/Receiver.h>
#include <Poco/Format.h>
#include <Poco/Logger.h>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <iostream>

int portNameToIndex(const std::string &name);

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
        workBump(false),
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
    bool workBump;
    bool activeState;
    WorkStats workStats;
    std::map<std::string, std::unique_ptr<InputPort>> inputs;
    std::map<std::string, std::unique_ptr<OutputPort>> outputs;

    //swap method that moves the internal state from another actor
    void swap(WorkerActor *oldActor)
    {
        std::swap(this->block, oldActor->block);
        std::swap(this->workBump, oldActor->workBump);
        std::swap(this->activeState, oldActor->activeState);
        std::swap(this->workStats, oldActor->workStats);
        std::swap(this->inputs, oldActor->inputs);
        std::swap(this->outputs, oldActor->outputs);
        for (auto &port : this->inputs) port.second->_actor = this;
        for (auto &port : this->outputs) port.second->_actor = this;
    }

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

    /*!
     * updatePorts() called after making changes to ports.
     * Reallocate and fill the indexed and named port structures.
     * Delete unsubscribed automatic ports.
     */
    void updatePorts(void);

    ///////////////////// convenience getters ///////////////////////
    OutputPort &getOutput(const std::string &name, const char *fcn);
    InputPort &getInput(const std::string &name, const char *fcn);
    OutputPort &getOutput(const size_t index, const char *fcn);
    InputPort &getInput(const size_t index, const char *fcn);

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

    std::string getInputBufferMode(const std::string &name, const std::string &domain)
    {
        try
        {
            if (block->getInputBufferManager(name, domain)) return "CUSTOM";
        }
        catch (const PortDomainError &)
        {
            return "ERROR";
        }
        return "ABDICATE";
    }

    std::string getOutputBufferMode(const std::string &name, const std::string &domain)
    {
        try
        {
            if (block->getOutputBufferManager(name, domain)) return "CUSTOM";
        }
        catch (const PortDomainError &)
        {
            return "ERROR";
        }
        return "ABDICATE";
    }

    BufferManager::Sptr getBufferManager(const std::string &name, const std::string &domain, const bool isInput)
    {
        auto m = isInput? block->getInputBufferManager(name, domain) : block->getOutputBufferManager(name, domain);
        if (not m) m = BufferManager::make("generic", BufferManagerArgs());
        else if (not m->isInitialized()) m->init(BufferManagerArgs()); //TODO pass this in from somewhere
        return m;
    }

    void setOutputBufferManager(const std::string &name, const BufferManager::Sptr &manager)
    {
        WorkerActorLock<WorkerActor> lock(this);
        outputs.at(name)->bufferManagerSetup(manager);
    }

    ///////////////////// work helper methods ///////////////////////
    inline void workTask(void)
    {
        if (not activeState) return;

        //prework
        {
            TimeAccumulator preWorkTime(workStats.totalTimePreWork);
            if (not this->preWorkTasks()) return;
        }

        //work
        POTHOS_EXCEPTION_TRY
        {
            workStats.numWorkCalls++;
            TimeAccumulator preWorkTime(workStats.totalTimeWork);
            block->work();
        }
        POTHOS_EXCEPTION_CATCH(const Exception &ex)
        {
            poco_error_f2(Poco::Logger::get("Pothos.Block.work"), "%s: %s", block->getName(), ex.displayText());
        }

        //postwork
        {
            TimeAccumulator preWorkTime(workStats.totalTimePostWork);
            this->postWorkTasks();
        }

        workStats.timeLastWork = std::chrono::high_resolution_clock::now();
    }
    bool preWorkTasks(void);
    void postWorkTasks(void);
};

/***********************************************************************
 * Inline port getter methods with nice exceptions
 **********************************************************************/
inline Pothos::OutputPort &Pothos::WorkerActor::getOutput(const std::string &name, const char *fcn)
{
    auto it = outputs.find(name);
    if (it == outputs.end()) throw PortAccessError(
        Poco::format("%s(%s)", std::string(fcn), name), "output port name out of range");
    return *it->second;
}

inline Pothos::InputPort &Pothos::WorkerActor::getInput(const std::string &name, const char *fcn)
{
    auto it = inputs.find(name);
    if (it == inputs.end()) throw PortAccessError(
        Poco::format("%s(%s)", std::string(fcn), name), "input port name out of range");
    return *it->second;
}

inline Pothos::OutputPort &Pothos::WorkerActor::getOutput(const size_t index, const char *fcn)
{
    auto &indexedOutputs = block->_indexedOutputs;
    if (index >= indexedOutputs.size()) throw PortAccessError(
        Poco::format("%s(%d)", std::string(fcn), int(index)), "output port index out of range");
    return *indexedOutputs[index];
}

inline Pothos::InputPort &Pothos::WorkerActor::getInput(const size_t index, const char *fcn)
{
    auto &indexedInputs = block->_indexedInputs;
    if (index >= indexedInputs.size()) throw PortAccessError(
        Poco::format("%s(%d)", std::string(fcn), int(index)), "input port index out of range");
    return *indexedInputs[index];
}
