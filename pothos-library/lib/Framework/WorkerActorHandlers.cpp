// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/WorkerActor.hpp"
#include <Poco/Format.h>
#include <Poco/Logger.h>
#include <algorithm> //sort
#include <cassert>

/*
void Pothos::WorkerActor::handleAsyncPortMessage(const PortMessage<InputPort *, TokenizedAsyncMessage> &message, const Theron::Address)
{
    assert(message.id != nullptr);
    auto &input = *message.id;
    auto &async = message.contents.async;
    if (input._impl->isSlot and async.type() == typeid(ObjectVector))
    {
        POTHOS_EXCEPTION_TRY
        {
            const auto &args = async.extract<ObjectVector>();
            block->opaqueCallHandler(input.name(), args.data(), args.size());
        }
        POTHOS_EXCEPTION_CATCH(const Exception &ex)
        {
            poco_error_f3(Poco::Logger::get("Pothos.Block.callSlot"), "%s[%s]: %s", block->getName(), input.name(), ex.displayText());
        }
        return this->bump();
    }
    if (input._impl->asyncMessages.full()) input._impl->asyncMessages.set_capacity(input._impl->asyncMessages.capacity()*2);
    input._impl->asyncMessages.push_back(message.contents);
    this->notify();
}

void Pothos::WorkerActor::handleInputBuffer(InputPort &input, const BufferChunk &buffer_)
{
    auto buffer = buffer_;
    if (not buffer.dtype or not input.dtype() or //unspecified
        (input.dtype().size() == buffer.dtype.size())) //size match
    {
        //unspecified buffer dtype? copy it from the port
        if (not buffer.dtype) buffer.dtype = input.dtype();
        input._impl->bufferAccumulator.push(buffer);
    }
    else
    {
        poco_error_f4(Poco::Logger::get("Pothos.Block.inputBuffer"), "%s[%s] dropped '%s', expected '%s'",
            block->getName(), input.name(), buffer.dtype.toString(), input.dtype().toString());
    }
}

void Pothos::WorkerActor::handleLabelsPortMessage(const PortMessage<InputPort *, LabeledBuffersMessage> &message, const Theron::Address)
{
    assert(message.id != nullptr);
    auto &input = *message.id;

    //insert labels (in order) and adjust for the current offset
    for (const auto &byteOffsetLabel : message.contents.labels)
    {
        auto label = byteOffsetLabel;
        auto elemSize = input.dtype().size();
        label.index += input._impl->bufferAccumulator.getTotalBytesAvailable(); //increment by enqueued bytes
        label.index /= elemSize; //convert from bytes to elements
        input._impl->inlineMessages.push_back(label);
    }

    //push all buffers into the accumulator
    for (const auto &buffer : message.contents.buffers) handleInputBuffer(input, buffer);

    this->notify();
}

void Pothos::WorkerActor::handleBufferPortMessage(const PortMessage<InputPort *, BufferChunk> &message, const Theron::Address)
{
    assert(message.id != nullptr);
    auto &input = *message.id;
    handleInputBuffer(input, message.contents);
    this->notify();
}
*/

void Pothos::WorkerActor::setOutputBufferManager(const std::string &name, const Pothos::BufferManager::Sptr &manager)
{
    WorkerActorLock<WorkerActor> lock(this);

    outputs.at(name)->_impl->bufferManagerSetup(manager);

    this->bump();
}
/*
void Pothos::WorkerActor::handleBufferReturnMessage(const BufferReturnMessage &message, const Theron::Address)
{
    auto mgr = message.buff.getBufferManager();
    if (mgr)
    {
        mgr->push(message.buff);
        this->notify();
    }
    else this->bump();
}
*/

void Pothos::WorkerActor::subscribePort(
    const std::string &action,
    const std::string &myPortName,
    Pothos::Block *subscriberPortBlock,
    const std::string &subscriberPortName)
{
    WorkerActorLock<WorkerActor> lock(this);

    //create the message
    PortSubscriber port;
    if (action.find("INPUT") != std::string::npos) port.inputPort = subscriberPortBlock->input(subscriberPortName);
    if (action.find("OUTPUT") != std::string::npos) port.outputPort = subscriberPortBlock->output(subscriberPortName);
    port.block = subscriberPortBlock;

    //extract the list of subscribers
    std::vector<PortSubscriber> *subscribers = nullptr;
    if (action.find("INPUT") != std::string::npos)
    {
        assert(port.inputPort != nullptr);
        auto &port = getOutput(myPortName, __FUNCTION__);
        subscribers = &port._impl->subscribers;
    }
    if (action.find("OUTPUT") != std::string::npos)
    {
        assert(port.outputPort != nullptr);
        auto &port = getInput(myPortName, __FUNCTION__);
        subscribers = &port._impl->subscribers;
    }
    assert(subscribers != nullptr);

    //locate the subscriber in the list
    auto sub = port;
    auto it = std::find(subscribers->begin(), subscribers->end(), sub);
    const bool found = it != subscribers->end();

    //subscriber is an input, add to the outputs subscribers list
    if (action == "SUBINPUT")
    {
        if (found) throw PortAccessError("Pothos::WorkerActor::handleSubscriberPortMessage("+action+")",
            Poco::format("input %s subscription exsists in output port %s", port.inputPort->name(), myPortName));
        subscribers->push_back(sub);
    }

    //subscriber is an output, add to the input subscribers list
    if (action == "SUBOUTPUT")
    {
        if (found) throw PortAccessError("Pothos::WorkerActor::handleSubscriberPortMessage("+action+")",
            Poco::format("output %s subscription exsists in input port %s", port.outputPort->name(), myPortName));
        subscribers->push_back(sub);
    }

    //unsubscriber is an input, remove from the outputs subscribers list
    if (action == "UNSUBINPUT")
    {
        if (not found) throw PortAccessError("Pothos::WorkerActor::handleSubscriberPortMessage("+action+")",
            Poco::format("input %s subscription missing from output port %s", port.inputPort->name(), myPortName));
        subscribers->erase(it);
    }

    //unsubscriber is an output, remove from the inputs subscribers list
    if (action == "UNSUBOUTPUT")
    {
        if (not found) throw PortAccessError("Pothos::WorkerActor::handleSubscriberPortMessage("+action+")",
            Poco::format("output %s subscription missing from input port %s", port.outputPort->name(), myPortName));
        subscribers->erase(it);
    }

    this->updatePorts();

    this->bump();
}
/*
void Pothos::WorkerActor::handleBumpWorkMessage(const BumpWorkMessage &, const Theron::Address)
{
    this->notify();
}
*/

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

    this->bump();
}

void Pothos::WorkerActor::setActiveStateOff(void)
{
    WorkerActorLock<WorkerActor> lock(this);

    //not activated? just return
    if (not this->activeState) return;

    this->activeState = false;
    this->block->deactivate();
}
/*
void Pothos::WorkerActor::handleShutdownActorMessage(const ShutdownActorMessage &message, const Theron::Address from)
{
    this->outputs.clear();
    this->inputs.clear();
    this->updatePorts();

    if (from != Theron::Address::Null()) this->Send(message, from);
}

void Pothos::WorkerActor::handleRequestPortInfoMessage(const RequestPortInfoMessage &message, const Theron::Address from)
{
    if (message.isInput) this->Send(getInput(message.name, __FUNCTION__).dtype(), from);
    else                 this->Send(getOutput(message.name, __FUNCTION__).dtype(), from);
    this->bump();
}

void Pothos::WorkerActor::handleRequestWorkerStatsMessage(const RequestWorkerStatsMessage &, const Theron::Address from)
{
    this->workStats.timeStatsQuery = std::chrono::high_resolution_clock::now();
    this->Send(this->workStats, from);
    this->bump();
}

void Pothos::WorkerActor::handleOpaqueCallMessage(const OpaqueCallMessage &message, const Theron::Address from)
{
    OpaqueCallResultMessage result;
    POTHOS_EXCEPTION_TRY
    {
        result.obj = this->block->opaqueCallHandler(message.name, message.inputArgs, message.numArgs);
    }
    POTHOS_EXCEPTION_CATCH(const Pothos::Exception &ex)
    {
        result.error.reset(ex.clone());
    }
    this->Send(result, from);
    this->bump();
}
*/
