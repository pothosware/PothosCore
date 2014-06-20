// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <Theron/Actor.h>
#include <Theron/Framework.h>
#include <string>

namespace Pothos
{
    class InputPort;
    class OutputPort;
}

struct PortSubscriber
{
    PortSubscriber(void):
        inputPort(nullptr),
        outputPort(nullptr)
    {}
    Pothos::InputPort *inputPort;
    Pothos::OutputPort *outputPort;
    Theron::Address address;
};

inline bool operator==(const PortSubscriber &lhs, const PortSubscriber &rhs)
{
    return lhs.inputPort == rhs.inputPort
        and lhs.outputPort == rhs.outputPort
        and lhs.address == rhs.address;
}

struct PortSubscriberMessage
{
    std::string action;
    PortSubscriber port;
};
