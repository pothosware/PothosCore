// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/WorkerActor.hpp"
#include <Poco/Format.h>
#include <algorithm> //sort
#include <cassert>

void Pothos::WorkerActor::handleAsyncPortNameMessage(const PortMessage<std::string, Object> &message, const Theron::Address)
{
    auto &input = getInput(message.id, __FUNCTION__);
    if (input._impl->asyncMessages.full()) input._impl->asyncMessages.set_capacity(input._impl->asyncMessages.capacity()*2);
    input._impl->asyncMessages.push_back(message.contents);
    this->notify();
}

void Pothos::WorkerActor::handleAsyncPortIndexMessage(const PortMessage<size_t, Object> &message, const Theron::Address)
{
    auto &input = getInput(message.id, __FUNCTION__);
    if (input._impl->asyncMessages.full()) input._impl->asyncMessages.set_capacity(input._impl->asyncMessages.capacity()*2);
    input._impl->asyncMessages.push_back(message.contents);
    this->notify();
}

void Pothos::WorkerActor::handleInlinePortNameMessage(const PortMessage<std::string, Label> &message, const Theron::Address)
{
    auto &input = getInput(message.id, __FUNCTION__);
    input._impl->inlineMessages.push_back(message.contents);
    std::sort(input._impl->inlineMessages.begin(), input._impl->inlineMessages.end());
    this->bump();
}

void Pothos::WorkerActor::handleInlinePortIndexMessage(const PortMessage<size_t, Label> &message, const Theron::Address)
{
    auto &input = getInput(message.id, __FUNCTION__);
    input._impl->inlineMessages.push_back(message.contents);
    std::sort(input._impl->inlineMessages.begin(), input._impl->inlineMessages.end());
    this->bump();
}

void Pothos::WorkerActor::handleBufferPortNameMessage(const PortMessage<std::string, BufferChunk> &message, const Theron::Address)
{
    auto &input = getInput(message.id, __FUNCTION__);
    input._impl->bufferAccumulator.push(message.contents);
    this->notify();
}

void Pothos::WorkerActor::handleBufferPortIndexMessage(const PortMessage<size_t, BufferChunk> &message, const Theron::Address)
{
    auto &input = getInput(message.id, __FUNCTION__);
    input._impl->bufferAccumulator.push(message.contents);
    this->notify();
}

void Pothos::WorkerActor::handleBufferReturnMessage(const BufferReturnMessage &message, const Theron::Address)
{
    message.mgr->push(message.buff);
    this->notify();
}

void Pothos::WorkerActor::handleSubscriberPortIndexMessage(const PortMessage<std::string, PortSubscriberMessage> &message, const Theron::Address from)
{
    try
    {
        //subscriber is an input, add to the outputs subscribers list
        if (message.contents.action == "SUBINPUT")
        {
            //indexed port does not exist, look for a lower index port and allocate
            //FIXME allocateOutput is adding to port names at the end, but this should be inserted at index(i)
            const int index = portNameToIndex(message.id);
            if (index != -1 and block->_outputs.count(message.id) == 0)
            {
                for (int i = index-1; i >= 0; i--)
                {
                    if (block->_outputs.count(std::to_string(i)) == 0) continue;
                    const auto dtype = getOutput(std::to_string(i), __FUNCTION__).dtype();
                    this->allocateOutput(message.id, dtype);
                    break;
                }
            }

            auto &port = getOutput(message.id, __FUNCTION__);
            auto sub = message.contents.port;
            auto it = std::find(port._impl->subscribers.begin(), port._impl->subscribers.end(), sub);
            if (it != port._impl->subscribers.end()) throw Poco::format("input %s subscription exsists in output port %s", sub.name, message.id);
            sub.index = portNameToIndex(sub.name);
            port._impl->subscribers.push_back(sub);
        }

        //subscriber is an output, add to the input subscribers list
        if (message.contents.action == "SUBOUTPUT")
        {
            //indexed port does not exist, look for a lower index port and allocate
            //FIXME allocateInput is adding to port names at the end, but this should be inserted at index(i)
            const int index = portNameToIndex(message.id);
            if (index != -1 and block->_inputs.count(message.id) == 0)
            {
                for (int i = index-1; i >= 0; i--)
                {
                    if (block->_inputs.count(std::to_string(i)) == 0) continue;
                    const auto dtype = getInput(std::to_string(i), __FUNCTION__).dtype();
                    this->allocateInput(message.id, dtype);
                    break;
                }
            }

            auto &port = getInput(message.id, __FUNCTION__);
            auto sub = message.contents.port;
            auto it = std::find(port._impl->subscribers.begin(), port._impl->subscribers.end(), sub);
            if (it != port._impl->subscribers.end()) throw Poco::format("output %s subscription exsists in input port %s", sub.name, message.id);
            sub.index = portNameToIndex(sub.name);
            port._impl->subscribers.push_back(sub);
        }

        //unsubscriber is an input, remove from the outputs subscribers list
        if (message.contents.action == "UNSUBINPUT")
        {
            auto &port = getOutput(message.id, __FUNCTION__);
            auto sub = message.contents.port;
            auto it = std::find(port._impl->subscribers.begin(), port._impl->subscribers.end(), sub);
            if (it == port._impl->subscribers.end()) throw Poco::format("input %s subscription missing from output port %s", sub.name, message.id);
            port._impl->subscribers.erase(it);
        }

        //unsubscriber is an output, remove from the inputs subscribers list
        if (message.contents.action == "UNSUBOUTPUT")
        {
            auto &port = getInput(message.id, __FUNCTION__);
            auto sub = message.contents.port;
            auto it = std::find(port._impl->subscribers.begin(), port._impl->subscribers.end(), sub);
            if (it == port._impl->subscribers.end()) throw Poco::format("output %s subscription missing from input port %s", sub.name, message.id);
            port._impl->subscribers.erase(it);
        }

        //TODO erase automatic ports that have 0 subscribers

        if (from != Theron::Address::Null()) this->Send(std::string(""), from);
    }
    catch (const Pothos::Exception &ex)
    {
        if (from != Theron::Address::Null()) this->Send(ex.displayText(), from);
    }
    catch (const std::string &ex)
    {
        if (from != Theron::Address::Null()) this->Send(ex, from);
    }
    this->bump();
}

void Pothos::WorkerActor::handleBumpWorkMessage(const BumpWorkMessage &, const Theron::Address)
{
    this->notify();
}

static void bufferManagerPushExternal(
    Pothos::BufferManager *mgr,
    std::shared_ptr<Theron::Framework> framework,
    const Theron::Address &addr,
    const Pothos::ManagedBuffer &buff
)
{
    BufferReturnMessage message;
    message.mgr = mgr;
    message.buff = buff;
    framework->Send(message, Theron::Address::Null(), addr);
}

void Pothos::WorkerActor::handleActivateWorkMessage(const ActivateWorkMessage &, const Theron::Address from)
{
    //setup the buffer return callback on the manager
    for (auto &entry : block->_outputs)
    {
        auto &port = *entry.second;
        auto &mgr = port._impl->bufferManager;
        mgr->setCallback(std::bind(&bufferManagerPushExternal,
            mgr.get(), block->_framework, this->GetAddress(), std::placeholders::_1));
    }

    try
    {
        this->block->activate();
        this->activeState = true;
        this->Send(std::string(""), from);
    }
    catch (const Pothos::Exception &ex)
    {
        this->Send(ex.displayText(), from);
    }
    catch (const Poco::Exception &ex)
    {
        this->Send(ex.displayText(), from);
    }
    catch (const std::runtime_error &ex)
    {
        this->Send(std::string(ex.what()), from);
    }
    catch (...)
    {
        this->Send(std::string("unknown error"), from);
    }
    this->bump();
}

void Pothos::WorkerActor::handleDeactivateWorkMessage(const DeactivateWorkMessage &, const Theron::Address from)
{
    try
    {
        this->activeState = false;
        this->block->deactivate();
        this->Send(std::string(""), from);
    }
    catch (const Pothos::Exception &ex)
    {
        this->Send(ex.displayText(), from);
    }
    catch (const Poco::Exception &ex)
    {
        this->Send(ex.displayText(), from);
    }
    catch (const std::runtime_error &ex)
    {
        this->Send(std::string(ex.what()), from);
    }
    catch (...)
    {
        this->Send(std::string("unknown error"), from);
    }
}

void Pothos::WorkerActor::handleShutdownActorMessage(const ShutdownActorMessage &message, const Theron::Address from)
{
    block->_outputs.clear();
    block->_inputs.clear();
    block->_indexedOutputs.clear();
    block->_indexedInputs.clear();
    block->_namedOutputs.clear();
    block->_namedInputs.clear();
    //TODO make a deallocate call for a port

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
    this->workStats.ticksStatsQuery = Theron::Detail::Clock::GetTicks();
    this->Send(this->workStats, from);
    this->bump();
}

void Pothos::WorkerActor::handleOpaqueCallMessage(const OpaqueCallMessage &message, const Theron::Address from)
{
    OpaqueCallResultMessage result;
    try
    {
        result.obj = this->block->opaqueCallHandler(message.name, message.inputArgs, message.numArgs);
    }
    catch (const Pothos::Exception &ex)
    {
        result.error.reset(ex.clone());
        //result.error.reset(new Pothos::Exception("Pothos::Block::call("+message.name+")", ex));
    }
    catch (const std::exception &ex)
    {
        result.error.reset(new Pothos::Exception("Pothos::Block::call("+message.name+")", ex.what()));
    }
    catch (...)
    {
        result.error.reset(new Pothos::Exception("Pothos::Block::call("+message.name+")", "unknown error"));
    }
    this->Send(result, from);
    this->bump();
}
