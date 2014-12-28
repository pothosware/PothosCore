// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/WorkerActor.hpp"
#include <Poco/Format.h>
#include <Poco/Logger.h>
#include <cassert>

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
}

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
