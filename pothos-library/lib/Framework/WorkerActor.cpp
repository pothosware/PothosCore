// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/WorkerActor.hpp"
#include <Poco/Logger.h>
#include <cassert>

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
            port._impl->readBeforeWritePort != nullptr and
            port.dtype().size() == port._impl->readBeforeWritePort->dtype().size() and
            (port._buffer = port._impl->readBeforeWritePort->_impl->bufferAccumulator.front()).useCount() == 2 //2 -> unique + this assignment
        ) port._impl->_bufferFromManager = false;
        else if (not port._impl->bufferManager or port._impl->bufferManager->empty())
        {
            port._buffer = BufferChunk();
            port._impl->_bufferFromManager = false;
        }
        else
        {
            port._buffer = port._impl->bufferManager->front();
            port._impl->_bufferFromManager = true;
        }
        port._elements = port._buffer.length/port.dtype().size();
        if (port._elements == 0) allOutputsReady = false;
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
        const size_t reserveBytes = port._reserveElements*port.dtype().size();
        port._impl->bufferAccumulator.require(reserveBytes);
        port._buffer = port._impl->bufferAccumulator.front();
        port._elements = port._buffer.get().length/port.dtype().size();
        if (port._elements < port._reserveElements) allInputsReady = false;
        if (not port._impl->asyncMessages.empty()) hasInputMessage = true;
        port._pendingElements = 0;
        port._labelIter = port._impl->inlineMessages;
        if (port.index() != -1)
        {
            assert(block->_workInfo.inputPointers.size() > size_t(port.index()));
            block->_workInfo.inputPointers[port.index()] = port._buffer.get().as<const void *>();
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

        //pop the consumed bytes from the accumulator
        if (bytes != 0) port._impl->bufferAccumulator.pop(bytes);

        //move consumed elements into total
        port._totalElements += port._pendingElements;

        //propagate labels and delete old
        size_t numLabels = 0;
        auto &allLabels = port._impl->inlineMessages;
        for (size_t i = 0; i < allLabels.size(); i++)
        {
            if (allLabels[i].index < port.totalElements()) numLabels++;
            else break;
        }
        if (numLabels != 0)
        {
            port._labelIter = LabelIteratorRange(allLabels.begin(), allLabels.begin()+numLabels);
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
    }

    //update consumption stats, bytes are incremental, messages cumulative
    if (bytesConsumed != 0 or this->workStats.msgsConsumed != msgsConsumed)
    {
        this->workStats.ticksLastConsumed = Theron::Detail::Clock::GetTicks();
    }
    this->workStats.bytesConsumed += bytesConsumed;
    msgsConsumed -= this->workStats.msgsConsumed;
    this->workStats.msgsConsumed += msgsConsumed;

    ///////////////////// output handling ////////////////////////
    //Note: output buffer production must come after propagateLabels()

    unsigned long long bytesProduced = 0;
    unsigned long long msgsProduced = 0;

    for (auto &entry : this->outputs)
    {
        auto &port = *entry.second;
        const size_t bytes = port._pendingElements*port.dtype().size();
        bytesProduced += bytes;
        msgsProduced += port._totalMessages;

        //set the buffer length, send it, pop from manager, clear reference
        if (bytes != 0)
        {
            auto &buffer = port._buffer;
            buffer.length = bytes;
            if (port._impl->_bufferFromManager) port._impl->bufferManager->pop(buffer.length);
            this->sendOutputPortMessage(port._impl->subscribers, buffer);
            port._buffer = BufferChunk(); //clear reference
        }

        //send the external buffers in the queue
        while (not port._impl->postedBuffers.empty())
        {
            auto &buffer = port._impl->postedBuffers.front();
            bytesProduced += buffer.length;
            port._totalElements += buffer.length/port.dtype().size();
            this->sendOutputPortMessage(port._impl->subscribers, buffer);
            port._impl->postedBuffers.pop_front();
        }

        //move produced elements into total
        port._totalElements += port._pendingElements;
    }

    //update production stats, bytes are incremental, messages cumulative
    if (bytesProduced != 0 or this->workStats.msgsProduced != msgsProduced)
    {
        this->workStats.ticksLastProduced = Theron::Detail::Clock::GetTicks();
    }
    this->workStats.bytesProduced += bytesProduced;
    msgsProduced -= this->workStats.msgsProduced;
    this->workStats.msgsProduced += msgsProduced;

    //postwork bump logic
    const bool hadConsumption = (bytesConsumed !=0 or msgsConsumed != 0);
    const bool hadProduction = (bytesProduced != 0 or msgsProduced != 0);
    if (this->workBump or hadConsumption or hadProduction) this->bump();
}

#include <Pothos/Managed.hpp>

static auto managedWorkerActor = Pothos::ManagedClass()
    .registerClass<Pothos::WorkerActor>()
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, sendActivateMessage))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, sendDeactivateMessage))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, sendPortSubscriberMessage))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, getInputBufferMode))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, getOutputBufferMode))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, getBufferManager))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, setOutputBufferManager))
    .commit("Pothos/WorkerActor");

static auto managedInfoReceiverString = Pothos::ManagedClass()
    .registerClass<InfoReceiver<std::string>>()
    .registerMethod(POTHOS_FCN_TUPLE(InfoReceiver<std::string>, make))
    .registerMethod(POTHOS_FCN_TUPLE(InfoReceiver<std::string>, WaitInfo))
    .commit("Pothos/InfoReceiverString");
