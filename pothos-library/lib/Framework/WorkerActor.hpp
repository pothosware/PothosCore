// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include "Framework/InputPortImpl.hpp"
#include "Framework/OutputPortImpl.hpp"
#include "Framework/WorkerStats.hpp"
#include <Pothos/Framework/BlockImpl.hpp>
#include <Pothos/Framework/Exception.hpp>
#include <Pothos/Object/Containers.hpp>
#include <Theron/Actor.h>
#include <Theron/Framework.h>
#include <Theron/Receiver.h>
#include <Poco/Format.h>
#include <Poco/Logger.h>
#include <iostream>

int portNameToIndex(const std::string &name);

/***********************************************************************
 * Helpful Theron::Receiver derived class for a specific value
 **********************************************************************/
template <typename InfoType>
class InfoReceiver : public Theron::Receiver
{
public:
    static std::shared_ptr<InfoReceiver<InfoType>> make(void)
    {
        return std::shared_ptr<InfoReceiver<InfoType>>(new InfoReceiver());
    }

    InfoReceiver(void)
    {
        this->RegisterHandler(this, &InfoReceiver::handle);
    }

    void handle(const InfoType &message, const Theron::Address)
    {
        this->_infos.push_back(message);
    }

    const InfoType &Info(void) const
    {
        return _infos.back();
    }

    const InfoType &WaitInfo(void)
    {
        this->Wait();
        return this->Info();
    }

    const std::vector<InfoType> &infos(void)
    {
        return _infos;
    }

private:
    std::vector<InfoType> _infos;
};

/***********************************************************************
 * Messages that will be sent to the actor
 **********************************************************************/
template <typename PortIdType, typename MessageType>
struct PortMessage
{
    PortIdType id;
    MessageType contents;
};

template <typename PortIdType, typename MessageType>
PortMessage<PortIdType, MessageType> makePortMessage(const PortIdType &id, const MessageType &contents)
{
    PortMessage<PortIdType, MessageType> message;
    message.id = id;
    message.contents = contents;
    return message;
}

struct LabeledBuffersMessage
{
    std::vector<Pothos::Label> labels;
    std::vector<Pothos::BufferChunk> buffers;
};

struct BufferReturnMessage
{
    Pothos::ManagedBuffer buff;
};

struct BumpWorkMessage
{
    //
};

struct ActivateWorkMessage
{
    //
};

struct DeactivateWorkMessage
{
    //
};

struct ShutdownActorMessage
{
    //
};

struct RequestPortInfoMessage
{
    bool isInput;
    std::string name;
};

struct RequestWorkerStatsMessage
{
    //
};

struct OpaqueCallMessage
{
    std::string name;
    const Pothos::Object *inputArgs;
    size_t numArgs;
};

struct OpaqueCallResultMessage
{
    Pothos::Object obj;
    std::shared_ptr<Pothos::Exception> error;
};

/***********************************************************************
 * Actor definition
 **********************************************************************/
class Pothos::WorkerActor : public Theron::Actor
{
public:
    WorkerActor(Block *block):
        Theron::Actor(*std::static_pointer_cast<Theron::Framework>(block->_threadPool.getContainer())),
        block(block),
        workBump(false),
        activeState(false)
    {
        this->RegisterHandler(this, &WorkerActor::handleAsyncPortMessage);
        this->RegisterHandler(this, &WorkerActor::handleLabelsPortMessage);
        this->RegisterHandler(this, &WorkerActor::handleBufferPortMessage);
        this->RegisterHandler(this, &WorkerActor::handleBufferReturnMessage);
        this->RegisterHandler(this, &WorkerActor::handleSubscriberPortMessage);
        this->RegisterHandler(this, &WorkerActor::handleBumpWorkMessage);
        this->RegisterHandler(this, &WorkerActor::handleActivateWorkMessage);
        this->RegisterHandler(this, &WorkerActor::handleDeactivateWorkMessage);
        this->RegisterHandler(this, &WorkerActor::handleShutdownActorMessage);
        this->RegisterHandler(this, &WorkerActor::handleRequestPortInfoMessage);
        this->RegisterHandler(this, &WorkerActor::handleRequestWorkerStatsMessage);
        this->RegisterHandler(this, &WorkerActor::handleOpaqueCallMessage);
    }

    inline void bump(void)
    {
        //only bump when we know there is nothing available in the queue
        if (this->GetNumQueuedMessages() == 1)
        {
            this->GetFramework().Send(BumpWorkMessage(), this->GetAddress(), this->GetAddress());
        }
    }

    ///////////////////// message handlers ///////////////////////
    void handleAsyncPortMessage(const PortMessage<InputPort *, Object> &message, const Theron::Address from);
    void handleLabelsPortMessage(const PortMessage<InputPort *, LabeledBuffersMessage> &message, const Theron::Address from);
    void handleBufferPortMessage(const PortMessage<InputPort *, BufferChunk> &message, const Theron::Address from);
    void handleBufferReturnMessage(const BufferReturnMessage &message, const Theron::Address from);
    void handleSubscriberPortMessage(const PortMessage<std::string, PortSubscriberMessage> &message, const Theron::Address from);
    void handleBumpWorkMessage(const BumpWorkMessage &message, const Theron::Address from);
    void handleActivateWorkMessage(const ActivateWorkMessage &message, const Theron::Address from);
    void handleDeactivateWorkMessage(const DeactivateWorkMessage &message, const Theron::Address from);
    void handleShutdownActorMessage(const ShutdownActorMessage &message, const Theron::Address from);
    void handleRequestPortInfoMessage(const RequestPortInfoMessage &message, const Theron::Address from);
    void handleRequestWorkerStatsMessage(const RequestWorkerStatsMessage &message, const Theron::Address from);
    void handleOpaqueCallMessage(const OpaqueCallMessage &message, const Theron::Address from);

    ///////////////////// send port messages ///////////////////////
    template <typename PortSubscribersType, typename MessageType>
    inline void sendInputPortMessage(const PortSubscribersType &subs, const MessageType &contents) const
    {
        assert(this != nullptr);
        for (const auto &s : subs)
        {
            assert(s.outputPort != nullptr);
            this->GetFramework().Send(makePortMessage(s.outputPort, contents), this->GetAddress(), s.block->_actor->GetAddress());
        }
    }
    template <typename PortSubscribersType, typename MessageType>
    inline void sendOutputPortMessage(const PortSubscribersType &subs, const MessageType &contents) const
    {
        assert(this != nullptr);
        for (const auto &s : subs)
        {
            assert(s.inputPort != nullptr);
            this->GetFramework().Send(makePortMessage(s.inputPort, contents), this->GetAddress(), s.block->_actor->GetAddress());
        }
    }

    ///////////////////// WorkerActor storage ///////////////////////
    Block *block;
    bool workBump;
    bool activeState;
    WorkerStats workStats;
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
        for (auto &port : this->inputs) port.second->_impl->actor = this;
        for (auto &port : this->outputs) port.second->_impl->actor = this;
    }

    ///////////////////// port setup methods ///////////////////////
    void allocateInput(const std::string &name, const DType &dtype, const std::string &domain);
    void allocateOutput(const std::string &name, const DType &dtype, const std::string &domain);
    void allocateSignal(const std::string &name);
    void allocateSlot(const std::string &name);
    template <typename ImplType, typename PortsType, typename PortNamesType>
    void allocatePort(PortsType &ports, PortNamesType &portNames, const std::string &name, const DType &dtype, const std::string &domain);

    void autoAllocateInput(const std::string &name);
    void autoAllocateOutput(const std::string &name);
    template <typename ImplType, typename PortsType, typename IndexedPortsType, typename PortNamesType>
    void autoAllocatePort(PortsType &ports, IndexedPortsType &indexedPorts, PortNamesType &portNames, const std::string &name);

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
    std::shared_ptr<InfoReceiver<std::string>> sendActivateMessage(void)
    {
        auto receiver = InfoReceiver<std::string>::make();
        this->GetFramework().Send(ActivateWorkMessage(), receiver->GetAddress(), this->GetAddress());
        return receiver;
    }

    std::shared_ptr<InfoReceiver<std::string>> sendDeactivateMessage(void)
    {
        auto receiver = InfoReceiver<std::string>::make();
        this->GetFramework().Send(DeactivateWorkMessage(), receiver->GetAddress(), this->GetAddress());
        return receiver;
    }

    std::shared_ptr<InfoReceiver<std::string>> sendPortSubscriberMessage(
        const std::string &action,
        const std::string &myPortName,
        Block *subscriberPortBlock,
        const std::string &subscriberPortName
    )
    {
        //create a new receiver to handle async reply
        auto receiver = InfoReceiver<std::string>::make();

        //create the message
        PortSubscriberMessage message;
        message.action = action;
        if (action.find("INPUT") != std::string::npos) message.port.inputPort = subscriberPortBlock->input(subscriberPortName);
        if (action.find("OUTPUT") != std::string::npos) message.port.outputPort = subscriberPortBlock->output(subscriberPortName);
        message.port.block = subscriberPortBlock;

        //send it to the actor
        this->GetFramework().Send(makePortMessage(myPortName, message), receiver->GetAddress(), this->GetAddress());
        return receiver;
    }

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
        else m->init(BufferManagerArgs()); //TODO pass this in from somewhere
        return m;
    }

    void setOutputBufferManager(const std::string &name, const BufferManager::Sptr &manager)
    {
        assert(manager);
        outputs[name]->_impl->bufferManager = manager;
    }

    ///////////////////// work helper methods ///////////////////////
    inline void notify(void)
    {
        //only call when we handle the only message in the actor's queue
        if (not activeState or this->GetNumQueuedMessages() > 1) return;

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
