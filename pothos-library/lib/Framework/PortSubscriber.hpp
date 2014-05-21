// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <Theron/Actor.h>
#include <Theron/Framework.h>
#include <string>

struct PortSubscriber
{
    PortSubscriber(void):
        index(-1)
    {}
    int index; //port number
    std::string name; //port name
    Theron::Address address;
};

inline bool operator==(const PortSubscriber &lhs, const PortSubscriber &rhs)
{
    return lhs.name == rhs.name and lhs.address == rhs.address;
}

struct PortSubscriberMessage
{
    std::string action;
    PortSubscriber port;
};
