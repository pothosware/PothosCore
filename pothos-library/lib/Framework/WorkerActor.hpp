// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include "Framework/ActorInterface.hpp"
#include <Pothos/Framework/BlockImpl.hpp>
#include <Pothos/Framework/Exception.hpp>
#include <Poco/Format.h>
#include <Poco/Logger.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <atomic>
#include <set>
#include <iostream>

/***********************************************************************
 * Actor definition
 **********************************************************************/
class Pothos::WorkerActor : public ActorInterface
{
public:
    WorkerActor(Block *block):
        block(block),
        activeState(false),
        activityIndicator(0),
        numTaskCalls(0),
        numWorkCalls(0)
    {
        return;
    }

    /*!
     * Perform the main processing task once.
     * Give the context back to the worker thread.
     */
    bool processTask(const bool waitEnabled)
    {
        if (this->workerThreadAcquire(waitEnabled))
        {
            this->workTask();
            this->workerThreadRelease();
            return true;
        }
        return false;
    }

    /*!
     * The activity indicator changes when work() produces or consumes.
     * Its value is used by the Topology's waitInactive() implementation.
     * While technically a counter, the value has no intrinsic meaning.
     * The Topology needs to be able to detect that activity occurred.
     */
    int queryActivityIndicator(void) const
    {
        return this->activityIndicator;
    }

    /*!
     * Query the work stats as a JSON object.
     * This call blocks the work thread context.
     * This call is made by the top level topology
     * to amalgamate stats from all blocks in the design.
     */
    Poco::JSON::Object::Ptr queryWorkStats(void);

    ///////////////////// WorkerActor storage ///////////////////////
    Block *block;
    bool activeState;
    std::atomic<int> activityIndicator;
    std::map<std::string, std::unique_ptr<InputPort>> inputs;
    std::map<std::string, std::unique_ptr<OutputPort>> outputs;

    ///////////////////// work stats collection ///////////////////////
    unsigned long long numTaskCalls;
    unsigned long long numWorkCalls;
    std::chrono::high_resolution_clock::duration totalTimeTask;
    std::chrono::high_resolution_clock::duration totalTimeWork;
    std::chrono::high_resolution_clock::duration totalTimePreWork;
    std::chrono::high_resolution_clock::duration totalTimePostWork;
    std::chrono::high_resolution_clock::time_point timeLastConsumed;
    std::chrono::high_resolution_clock::time_point timeLastProduced;
    std::chrono::high_resolution_clock::time_point timeLastWork;

    ///////////////////// port setup methods ///////////////////////
    void allocateInput(const std::string &name, const DType &dtype, const std::string &domain);
    void allocateOutput(const std::string &name, const DType &dtype, const std::string &domain);
    void allocateSignal(const std::string &name);
    void allocateSlot(const std::string &name);
    template <typename PortsType, typename NamedPortsType, typename IndexedPortsType, typename PortNamesType>
    void allocatePort(PortsType &ports, NamedPortsType &namedPorts, IndexedPortsType &indexedPorts, PortNamesType &portNames,
        const std::string &name, const DType &dtype, const std::string &domain, const bool automatic = false);

    void autoAllocateInput(const std::string &name);
    void autoAllocateOutput(const std::string &name);
    template <typename PortsType, typename NamedPortsType, typename IndexedPortsType, typename PortNamesType>
    void autoAllocatePort(PortsType &ports, NamedPortsType &namedPorts, IndexedPortsType &indexedPorts, PortNamesType &portNames, const std::string &name);

    template <typename PortsType, typename NamedPortsType, typename IndexedPortsType, typename PortNamesType>
    void autoDeletePort(const std::string &name, PortsType &ports, NamedPortsType &namedPorts, IndexedPortsType &indexedPorts, PortNamesType &portNames);
    std::set<void *> automaticPorts; //set of automatically allocated ports
    void autoDeleteInput(const std::string &name);
    void autoDeleteOutput(const std::string &name);

    //! call after making changes to ports
    void updatePorts(void);

    ///////////////////// topology helper methods ///////////////////////
    void setActiveStateOn(void);
    void setActiveStateOff(void);
    void subscribeInput(const std::string &action, const std::string &myPortName, InputPort *subscriberPort);
    void subscribeOutput(const std::string &action, const std::string &myPortName, OutputPort *subscriberPort);
    std::string getInputBufferMode(const std::string &name, const std::string &domain);
    std::string getOutputBufferMode(const std::string &name, const std::string &domain);
    BufferManager::Sptr getBufferManager(const std::string &name, const std::string &domain, const bool isInput);
    void setOutputBufferManager(const std::string &name, const BufferManager::Sptr &manager);

    ///////////////////// work helper methods ///////////////////////
    void workTask(void);
    bool preWorkTasks(void);
    void postWorkTasks(void);
};
