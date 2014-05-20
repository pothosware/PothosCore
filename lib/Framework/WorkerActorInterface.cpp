// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/WorkerActor.hpp"
#include <Poco/URI.h>
#include <Poco/Environment.h>
#include <Poco/Process.h>
#include <Poco/Format.h>
#include <memory>
#include <iostream>

struct WorkerActorInterface
{
    WorkerActorInterface(std::shared_ptr<Pothos::WorkerActor> actor):
        actor(actor)
    {
        return;
    }

    Theron::Address getAddress(void) const
    {
        return actor->GetAddress();
    }

    std::string upid(void) const
    {
        return Poco::URI("pothos", Poco::Environment::nodeName(), Poco::format(
            "%s/%s",
            Poco::Environment::nodeId(),
            std::to_string(Poco::Process::id())
        )).toString();
    }

    void sendActivateMessage(void)
    {
        receiver.reset(new InfoReceiver<std::string>());
        actor->GetFramework().Send(ActivateWorkMessage(), receiver->GetAddress(), actor->GetAddress());
    }

    void sendDeactivateMessage(void)
    {
        receiver.reset(new InfoReceiver<std::string>());
        actor->GetFramework().Send(DeactivateWorkMessage(), receiver->GetAddress(), actor->GetAddress());
    }

    void sendPortSubscriberMessage(
        const std::string &action,
        const std::string &myPortName,
        const std::string &subscriberPortName,
        const Theron::Address &subscriberPortAddr
    )
    {
        //create a new receiver to handle async reply
        receiver.reset(new InfoReceiver<std::string>());

        //create the message
        PortSubscriberMessage message;
        message.action = action;
        message.port.name = subscriberPortName;
        message.port.address = subscriberPortAddr;

        //send it to the actor
        actor->GetFramework().Send(makePortMessage(myPortName, message), receiver->GetAddress(), actor->GetAddress());
    }

    std::string waitStringResult(void)
    {
        assert(receiver);
        return receiver->WaitInfo();
    }

    Pothos::DType getPortDType(const bool isInput, const std::string name)
    {
        RequestPortInfoMessage message;
        message.isInput = isInput;
        message.name = name;

        InfoReceiver<Pothos::PortInfo> receiver;
        actor->GetFramework().Send(message, receiver.GetAddress(), actor->GetAddress());

        return receiver.WaitInfo().dtype();
    }

    WorkerStats getWorkerStats(void)
    {
        InfoReceiver<WorkerStats> receiver;
        actor->GetFramework().Send(RequestWorkerStatsMessage(), receiver.GetAddress(), actor->GetAddress());
        return receiver.WaitInfo();
    }

    std::shared_ptr<Pothos::WorkerActor> actor;
    std::shared_ptr<InfoReceiver<std::string>> receiver;
};

#include <Pothos/Managed.hpp>

static auto managedWorkerActorInterface = Pothos::ManagedClass()
    .registerConstructor<WorkerActorInterface, std::shared_ptr<Pothos::WorkerActor>>()
    .registerMethod(POTHOS_FCN_TUPLE(WorkerActorInterface, getAddress))
    .registerMethod(POTHOS_FCN_TUPLE(WorkerActorInterface, upid))
    .registerMethod(POTHOS_FCN_TUPLE(WorkerActorInterface, sendActivateMessage))
    .registerMethod(POTHOS_FCN_TUPLE(WorkerActorInterface, sendDeactivateMessage))
    .registerMethod(POTHOS_FCN_TUPLE(WorkerActorInterface, sendPortSubscriberMessage))
    .registerMethod(POTHOS_FCN_TUPLE(WorkerActorInterface, waitStringResult))
    .registerMethod(POTHOS_FCN_TUPLE(WorkerActorInterface, getPortDType))
    .registerMethod(POTHOS_FCN_TUPLE(WorkerActorInterface, getWorkerStats))
    .commit("Pothos/WorkerActorInterface");
