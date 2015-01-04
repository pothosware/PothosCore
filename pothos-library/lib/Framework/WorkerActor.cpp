// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/WorkerActor.hpp"
#include <Poco/Format.h>
#include <Poco/Logger.h>
#include <cassert>

/***********************************************************************
 * port subscribe/unsubscribe
 **********************************************************************/
void Pothos::WorkerActor::subscribePort(
    const std::string &myPortName,
    Pothos::Block *subscriberPortBlock,
    const std::string &subscriberPortName)
{
    WorkerActorLock<WorkerActor> lock(this);

    auto inputPort = subscriberPortBlock->input(subscriberPortName);
    auto &subscribers = getOutput(myPortName, __FUNCTION__)._subscribers;

    //locate the subscriber in the list
    auto it = std::find(subscribers.begin(), subscribers.end(), inputPort);
    const bool found = it != subscribers.end();

    //add to the outputs subscribers list
    if (found) throw PortAccessError("Pothos::WorkerActor::subscribePort()",
        Poco::format("input %s subscription exists in output port %s", inputPort->name(), myPortName));
    subscribers.push_back(inputPort);

    this->updatePorts();
}

void Pothos::WorkerActor::unsubscribePort(
    const std::string &myPortName,
    Pothos::Block *subscriberPortBlock,
    const std::string &subscriberPortName)
{
    WorkerActorLock<WorkerActor> lock(this);

    auto inputPort = subscriberPortBlock->input(subscriberPortName);
    auto &subscribers = getOutput(myPortName, __FUNCTION__)._subscribers;

    //locate the subscriber in the list
    auto it = std::find(subscribers.begin(), subscribers.end(), inputPort);
    const bool found = it != subscribers.end();

    //remove from the outputs subscribers list
    if (not found) throw PortAccessError("Pothos::WorkerActor::unsubscribePort()",
        Poco::format("input %s subscription missing from output port %s", inputPort->name(), myPortName));
    subscribers.erase(it);

    this->updatePorts();
}

/***********************************************************************
 * activate/deactivate
 **********************************************************************/
void Pothos::WorkerActor::setActiveStateOn(void)
{
    WorkerActorLock<WorkerActor> lock(this);

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
    WorkerActorLock<WorkerActor> lock(this);

    //not activated? just return
    if (not this->activeState) return;

    this->activeState = false;
    this->block->deactivate();
}

/***********************************************************************
 * pre-work
 **********************************************************************/
bool Pothos::WorkerActor::preWorkTasks(void)
{
    this->workBump = false;
    const size_t BIG = (1 << 30);

    bool allOutputsReady = true;
    bool allInputsReady = true;
    bool hasInputMessage = false;

    //////////////// output state calculation ///////////////////
    block->_workInfo.minOutElements = BIG;
    block->_workInfo.minAllOutElements = BIG;
    for (auto &entry : this->outputs)
    {
        auto &port = *entry.second;
        if ( //handle read before wite port if specified
            port._readBeforeWritePort != nullptr and
            port.dtype().size() == port._readBeforeWritePort->dtype().size() and
            (port._buffer = port._readBeforeWritePort->bufferAccumulatorFront()).useCount() == 2 //2 -> accumulator + this assignment
        ) port._bufferFromManager = false;
        else if (port.bufferManagerEmpty())
        {
            port._buffer = BufferChunk::null();
            port._bufferFromManager = false;
        }
        else
        {
            port._buffer = port.bufferManagerFront();
            port._bufferFromManager = true;
        }
        port._buffer.dtype = port.dtype(); //always copy from port's dtype setting
        assert(not port._bufferFromManager or port._buffer == port.bufferManagerFront());
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
    for (auto &entry : this->inputs)
    {
        auto &port = *entry.second;
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
        port._buffer = port.bufferAccumulatorFront();
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

    unsigned long long bytesConsumed = 0;
    unsigned long long msgsConsumed = 0;

    for (auto &entry : this->inputs)
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
        }

        //pop the consumed bytes from the accumulator
        if (bytes != 0)
        {
            if (bytes > port.bufferAccumulatorTotalBytes())
            {
                poco_error_f4(Poco::Logger::get("Pothos.Block.consume"), "%s[%s] overconsumed %d bytes, %d available",
                    block->getName(), port.name(), int(bytes), int(port.bufferAccumulatorTotalBytes()));
            }
            else port.bufferAccumulatorPop(bytes, port._pendingElements);
        }
        port._buffer = BufferChunk::null(); //clear reference

        //move consumed elements into total
        port._totalElements += port._pendingElements;
    }

    //update consumption stats, bytes are incremental, messages cumulative
    this->workStats.bytesConsumed += bytesConsumed;
    msgsConsumed -= this->workStats.msgsConsumed;
    this->workStats.msgsConsumed += msgsConsumed;
    const bool hadConsumption = (bytesConsumed !=0 or msgsConsumed != 0);
    if (hadConsumption) this->workStats.timeLastConsumed = std::chrono::high_resolution_clock::now();

    ///////////////////// output handling ////////////////////////
    //Note: output buffer production must come after propagateLabels()

    unsigned long long bytesProduced = 0;
    unsigned long long msgsProduced = 0;

    for (auto &entry : this->outputs)
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
            subscriber->_actor->flagChange();
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
    }

    //update production stats, bytes are incremental, messages cumulative
    this->workStats.bytesProduced += bytesProduced;
    msgsProduced -= this->workStats.msgsProduced;
    this->workStats.msgsProduced += msgsProduced;
    const bool hadProduction = (bytesProduced != 0 or msgsProduced != 0);
    if (hadProduction) this->workStats.timeLastProduced = std::chrono::high_resolution_clock::now();

    //postwork bump logic
    if (this->workBump or hadConsumption or hadProduction) this->flagChange();
}

#include <Pothos/Managed.hpp>

static auto managedWorkerActor = Pothos::ManagedClass()
    .registerClass<Pothos::WorkerActor>()
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, setActiveStateOn))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, setActiveStateOff))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, subscribePort))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, unsubscribePort))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, getInputBufferMode))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, getOutputBufferMode))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, getBufferManager))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, setOutputBufferManager))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, autoAllocateInput))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, autoAllocateOutput))
    .commit("Pothos/WorkerActor");
