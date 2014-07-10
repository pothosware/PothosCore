// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <string>

namespace Pothos
{
    class InputPort;
    class OutputPort;
    class Block;
}

struct PortSubscriber
{
    PortSubscriber(void):
        inputPort(nullptr),
        outputPort(nullptr),
        block(nullptr)
    {}
    Pothos::InputPort *inputPort;
    Pothos::OutputPort *outputPort;
    const Pothos::Block *block;
};

inline bool operator==(const PortSubscriber &lhs, const PortSubscriber &rhs)
{
    return lhs.inputPort == rhs.inputPort
        and lhs.outputPort == rhs.outputPort
        and lhs.block == rhs.block;
}

struct PortSubscriberMessage
{
    std::string action;
    PortSubscriber port;
};
