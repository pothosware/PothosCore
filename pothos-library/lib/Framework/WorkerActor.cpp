// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/WorkerActor.hpp"
#include <Pothos/Framework/InputPortImpl.hpp>
#include <Pothos/Framework/OutputPortImpl.hpp>
#include <Pothos/Object/Containers.hpp>
#include <Poco/Format.h>
#include <Poco/Logger.h>
#include <cassert>

/***********************************************************************
 * buffer manager helpers
 **********************************************************************/
std::string Pothos::WorkerActor::getInputBufferMode(const std::string &name, const std::string &domain)
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

std::string Pothos::WorkerActor::getOutputBufferMode(const std::string &name, const std::string &domain)
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

Pothos::BufferManager::Sptr Pothos::WorkerActor::getBufferManager(const std::string &name, const std::string &domain, const bool isInput)
{
    auto m = isInput? block->getInputBufferManager(name, domain) : block->getOutputBufferManager(name, domain);
    if (not m) m = BufferManager::make("generic", BufferManagerArgs());
    else if (not m->isInitialized()) m->init(BufferManagerArgs()); //TODO pass this in from somewhere
    return m;
}

void Pothos::WorkerActor::setOutputBufferManager(const std::string &name, const BufferManager::Sptr &manager)
{
    ActorInterfaceLock lock(this);
    outputs.at(name)->bufferManagerSetup(manager);
}

/***********************************************************************
 * port subscribe/unsubscribe
 **********************************************************************/
void Pothos::WorkerActor::subscribeInput(const std::string &action, const std::string &myPortName, Pothos::InputPort *inputPort)
{
    ActorInterfaceLock lock(this);

    //locate the subscriber in the list
    auto &subscribers = this->outputs.at(myPortName)->_subscribers;
    auto it = std::find(subscribers.begin(), subscribers.end(), inputPort);
    const bool found = it != subscribers.end();

    if (action == "add") //add to the output port's subscribers list
    {
        if (found) throw PortAccessError("Pothos::WorkerActor::subscribePort()",
            Poco::format("input %s subscription exists in output port %s", inputPort->name(), myPortName));
        subscribers.push_back(inputPort);
    }
    if (action == "remove") //remove from the output port's subscribers list
    {
        if (not found) throw PortAccessError("Pothos::WorkerActor::unsubscribePort()",
            Poco::format("input %s subscription missing from output port %s", inputPort->name(), myPortName));
        subscribers.erase(it);
    }

    this->updatePorts();
}

void Pothos::WorkerActor::subscribeOutput(const std::string &action, const std::string &myPortName, Pothos::OutputPort *outputPort)
{
    ActorInterfaceLock lock(this);

    //locate the subscriber in the list
    auto &subscribers = this->inputs.at(myPortName)->_subscribers;
    auto it = std::find(subscribers.begin(), subscribers.end(), outputPort);
    const bool found = it != subscribers.end();

    if (action == "add") //add to the input port's subscribers list
    {
        if (found) throw PortAccessError("Pothos::WorkerActor::subscribePort()",
            Poco::format("output %s subscription exists in input port %s", outputPort->name(), myPortName));
        subscribers.push_back(outputPort);
    }
    if (action == "remove") //remove from the input port's subscribers list
    {
        if (not found) throw PortAccessError("Pothos::WorkerActor::unsubscribePort()",
            Poco::format("output %s subscription missing from input port %s", outputPort->name(), myPortName));
        subscribers.erase(it);
    }

    this->updatePorts();
}

/***********************************************************************
 * activate/deactivate
 **********************************************************************/
void Pothos::WorkerActor::setActiveStateOn(void)
{
    ActorInterfaceLock lock(this);

    POTHOS_EXCEPTION_TRY
    {
        this->activeState = true;
        this->block->activate();
    }
    POTHOS_EXCEPTION_CATCH(const Exception &ex)
    {
        this->activeState = false;
        throw ex;
    }
}

void Pothos::WorkerActor::setActiveStateOff(void)
{
    ActorInterfaceLock lock(this);

    //not activated? just return
    if (not this->activeState) return;

    this->activeState = false;
    this->block->deactivate();
}

/***********************************************************************
 * work task dispatcher
 **********************************************************************/
void Pothos::WorkerActor::workTask(void)
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

/***********************************************************************
 * pre-work
 **********************************************************************/
bool Pothos::WorkerActor::preWorkTasks(void)
{
    const size_t BIG = (1 << 30);

    bool allOutputsReady = true;
    bool allInputsReady = true;
    bool hasInputMessage = false;

    //////////////// output state calculation ///////////////////
    block->_workInfo.minOutElements = BIG;
    block->_workInfo.minAllOutElements = BIG;
    for (const auto &entry : this->outputs)
    {
        auto &port = *entry.second;
        port._workEvents = 0;

        //is it ok to use the read-before-write optimization?
        const auto tryRBW = port._readBeforeWritePort != nullptr and
        port.dtype().size() == port._readBeforeWritePort->dtype().size();
        if (tryRBW)
        {
            port._readBeforeWritePort->_buffer = BufferChunk::null();
            port._readBeforeWritePort->bufferAccumulatorFront(port._buffer);
        }

        //now determine the buffer provided to this port
        if (tryRBW and port._buffer.useCount() == 2) //2 -> accumulator + this port
        {
            port._bufferFromManager = false;
        }
        else if (port.bufferManagerEmpty())
        {
            port._buffer = BufferChunk::null();
            port._bufferFromManager = false;
        }
        else
        {
            port.bufferManagerFront(port._buffer);
            port._bufferFromManager = true;
        }
        port._buffer.dtype = port.dtype(); //always copy from port's dtype setting
        port._elements = port._buffer.elements();
        if (port._elements == 0 and not port.isSignal()) allOutputsReady = false;
        if (port.tokenManagerEmpty()) allOutputsReady = false;
        port._pendingElements = 0;
        if (port.index() != -1)
        {
            assert(block->_workInfo.outputPointers.size() > size_t(port.index()));
            block->_workInfo.outputPointers[port.index()] = port._buffer.as<void *>();
            block->_workInfo.minOutElements = std::min(block->_workInfo.minOutElements, port._elements);
        }
        block->_workInfo.minAllOutElements = std::min(block->_workInfo.minAllOutElements, port._elements);
    }

    //////////////// input state calculation ///////////////////
    block->_workInfo.minInElements = BIG;
    block->_workInfo.minAllInElements = BIG;
    for (const auto &entry : this->inputs)
    {
        auto &port = *entry.second;
        port._workEvents = 0;
        if (not port.slotCallsEmpty())
        {
            POTHOS_EXCEPTION_TRY
            {
                const auto args =  port.slotCallsPop().extract<ObjectVector>();
                block->opaqueCallHandler(port.name(), args.data(), args.size());
            }
            POTHOS_EXCEPTION_CATCH(const Exception &ex)
            {
                poco_error_f3(Poco::Logger::get("Pothos.Block.callSlot"), "%s[%s]: %s", block->getName(), port.name(), ex.displayText());
            }
        }
        //perform minimum reserve accumulator require to recover from possible element fragmentation
        const size_t requireElems = std::max<size_t>(1, port._reserveElements);
        port.bufferAccumulatorRequire(requireElems*port.dtype().size());
        port.bufferAccumulatorFront(port._buffer);
        port._elements = port._buffer.length/port.dtype().size();
        if (port._elements < port._reserveElements) allInputsReady = false;
        if (not port.asyncMessagesEmpty()) hasInputMessage = true;
        port._pendingElements = 0;
        port._labelIter = port._inlineMessages;
        if (port.index() != -1)
        {
            assert(block->_workInfo.inputPointers.size() > size_t(port.index()));
            block->_workInfo.inputPointers[port.index()] = port._buffer.as<const void *>();
            block->_workInfo.minInElements = std::min(block->_workInfo.minInElements, port._elements);
        }
        block->_workInfo.minAllInElements = std::min(block->_workInfo.minAllInElements, port._elements);
    }

    //calculate overall minimums
    block->_workInfo.minElements = std::min(block->_workInfo.minInElements, block->_workInfo.minOutElements);
    block->_workInfo.minAllElements = std::min(block->_workInfo.minAllInElements, block->_workInfo.minAllOutElements);

    //arbitrary time, but its small
    block->_workInfo.maxTimeoutNs = 1000000; //1 millisecond

    return allOutputsReady and (allInputsReady or hasInputMessage);
}

/***********************************************************************
 * post-work
 **********************************************************************/
void Pothos::WorkerActor::postWorkTasks(void)
{
    ///////////////////// input handling ////////////////////////

    size_t inputWorkEvents = 0;
    unsigned long long bytesConsumed = 0;
    unsigned long long msgsConsumed = 0;

    for (const auto &entry : this->inputs)
    {
        auto &port = *entry.second;
        const size_t bytes = port._pendingElements*port.dtype().size();
        bytesConsumed += bytes;
        msgsConsumed += port._totalMessages;

        //propagate labels and delete old
        size_t numLabels = 0;
        auto &allLabels = port._inlineMessages;
        for (size_t i = 0; i < allLabels.size(); i++)
        {
            if (allLabels[i].index < port._pendingElements) numLabels++;
            //adjust labels index for new relative position
            else allLabels[i].index -= port._pendingElements;
        }

        if (numLabels != 0)
        {
            port._labelIter = LabelIteratorRange(allLabels.data(), allLabels.data()+numLabels);
            POTHOS_EXCEPTION_TRY
            {
                block->propagateLabels(&port);
            }
            POTHOS_EXCEPTION_CATCH(const Exception &ex)
            {
                poco_error_f2(Poco::Logger::get("Pothos.Block.propagateLabels"), "%s: %s", block->getName(), ex.displayText());
            }

            allLabels.erase(allLabels.begin(), allLabels.begin()+numLabels);
            port._totalLabels += numLabels;
        }

        //pop the consumed bytes from the accumulator
        if (bytes != 0)
        {
            port.bufferAccumulatorPop(bytes);
        }
        port._buffer = BufferChunk::null(); //clear reference

        //move consumed elements into total
        port._totalElements += port._pendingElements;
        inputWorkEvents += port._workEvents;
    }

    //update consumption stats, bytes are incremental, messages cumulative
    this->workStats.bytesConsumed += bytesConsumed;
    msgsConsumed -= this->workStats.msgsConsumed;
    this->workStats.msgsConsumed += msgsConsumed;
    if (inputWorkEvents != 0)
    {
        this->flagInternalChange();
        this->activityIndicator.fetch_add(1, std::memory_order_relaxed);
        this->workStats.timeLastConsumed = std::chrono::high_resolution_clock::now();
    }

    ///////////////////// output handling ////////////////////////
    //Note: output buffer production must come after propagateLabels()

    size_t outputWorkEvents = 0;
    unsigned long long bytesProduced = 0;
    unsigned long long msgsProduced = 0;

    for (const auto &entry : this->outputs)
    {
        auto &port = *entry.second;
        msgsProduced += port._totalMessages;
        size_t elemsDequeued = 0;
        size_t bytesDequeued = 0;

        //set the buffer length, send it, pop from manager, clear reference
        const size_t pendingBytes = port._pendingElements*port.buffer().dtype.size();
        if (pendingBytes != 0)
        {
            auto &buffer = port._buffer;
            buffer.length = pendingBytes;
            if (port._bufferFromManager)
            {
                if (buffer.length > buffer.getBuffer().getLength())
                {
                    poco_error_f4(Poco::Logger::get("Pothos.Block.produce"), "%s[%s] overproduced %d bytes, %d available",
                        block->getName(), port.name(), int(buffer.length), int(buffer.getBuffer().getLength()));
                }
                else port.bufferManagerPop(buffer.length);
            }
            port.postBuffer(buffer);
        }
        port._buffer = BufferChunk::null(); //clear reference

        //sort the posted labels in case the user posted out of order
        auto &postedLabels = port._postedLabels;
        auto &postedBuffers = port._postedBuffers;
        if (not postedLabels.empty()) std::sort(postedLabels.begin(), postedLabels.end());

        //send the outgoing labels with buffers
        for (const auto &subscriber : port._subscribers)
        {
            subscriber->bufferLabelPush(postedLabels, postedBuffers);
        }

        //clear posted labels
        postedLabels.clear();

        //clear posted buffers and save stats
        while (not postedBuffers.empty())
        {
            auto &buffer = postedBuffers.front();
            elemsDequeued += buffer.elements();
            bytesDequeued += buffer.length;
            postedBuffers.pop_front();
        }

        //add produced bytes into total
        port._totalElements += elemsDequeued;
        bytesProduced += bytesDequeued;
        outputWorkEvents += port._workEvents;
    }

    //update production stats, bytes are incremental, messages cumulative
    this->workStats.bytesProduced += bytesProduced;
    msgsProduced -= this->workStats.msgsProduced;
    this->workStats.msgsProduced += msgsProduced;
    if (outputWorkEvents != 0)
    {
        this->flagInternalChange();
        this->activityIndicator.fetch_add(1, std::memory_order_relaxed);
        this->workStats.timeLastProduced = std::chrono::high_resolution_clock::now();
    }
}

#include <Pothos/Managed.hpp>

static auto managedWorkerActor = Pothos::ManagedClass()
    .registerClass<Pothos::WorkerActor>()
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, setActiveStateOn))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, setActiveStateOff))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, subscribeInput))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, subscribeOutput))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, getInputBufferMode))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, getOutputBufferMode))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, getBufferManager))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, setOutputBufferManager))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, autoAllocateInput))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, autoAllocateOutput))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, autoDeleteInput))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, autoDeleteOutput))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, queryActivityIndicator))
    .commit("Pothos/WorkerActor");
