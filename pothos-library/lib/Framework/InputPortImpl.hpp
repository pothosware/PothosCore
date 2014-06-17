// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include "Framework/PortSubscriber.hpp"
#include <Pothos/Framework/InputPort.hpp>
#include <Pothos/Framework/BufferManager.hpp>
#include <Pothos/Framework/BufferAccumulator.hpp>
#include <Pothos/Util/RingDeque.hpp>
#include <vector>

class Pothos::InputPortImpl
{
public:
    InputPortImpl(void):
        actor(nullptr),
        isSlot(false){}
    BufferManager::Sptr bufferManager;
    Util::RingDeque<Object> asyncMessages;
    std::vector<Label> inlineMessages;
    BufferAccumulator bufferAccumulator;
    std::vector<PortSubscriber> subscribers;
    WorkerActor *actor;
    bool isSlot;
};
