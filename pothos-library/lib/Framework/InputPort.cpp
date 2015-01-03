// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/InputPort.hpp>
#include "Framework/WorkerActor.hpp"

Pothos::InputPort::InputPort(void):
    _actor(nullptr),
    _index(-1),
    _buffer(BufferChunk::null()),
    _elements(0),
    _totalElements(0),
    _totalMessages(0),
    _pendingElements(0),
    _reserveElements(0),
    _isSlot(false)
{
    return;
}

Pothos::InputPort::~InputPort(void)
{
    return;
}

Pothos::Object Pothos::InputPort::popMessage(void)
{
    if (_asyncMessages.empty()) return Pothos::Object();
    auto msg = _asyncMessages.front().first;
    _asyncMessages.pop_front();
    _totalMessages++;
    _actor->workBump = true;
    return msg;
}

void Pothos::InputPort::removeLabel(const Label &label)
{
    for (auto it = _inlineMessages.begin(); it != _inlineMessages.end(); it++)
    {
        if (*it == label)
        {
            _inlineMessages.erase(it);
            _labelIter = _inlineMessages;
            _actor->workBump = true;
            return;
        }
    }
}

void Pothos::InputPort::setReserve(const size_t numElements)
{
    _reserveElements = numElements;
    _actor->workBump = true;
}

void Pothos::InputPort::pushBuffer(const BufferChunk &buffer)
{
    _actor->GetFramework().Send(makePortMessage(this, buffer), _actor->GetAddress(), _actor->GetAddress());
}

void Pothos::InputPort::pushLabel(const Label &label)
{
    _actor->GetFramework().Send(makePortMessage(this, label), _actor->GetAddress(), _actor->GetAddress());
}

void Pothos::InputPort::pushMessage(const Object &message)
{
    _actor->GetFramework().Send(makePortMessage(this, message), _actor->GetAddress(), _actor->GetAddress());
}

void Pothos::InputPort::clear(void)
{
    _bufferAccumulator = BufferAccumulator();
    _inlineMessages.clear();
    _asyncMessages.clear();
}

#include <Pothos/Managed.hpp>

static auto managedInputPort = Pothos::ManagedClass()
    .registerClass<Pothos::InputPort>()
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::InputPort, index))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::InputPort, name))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::InputPort, dtype))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::InputPort, domain))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::InputPort, buffer))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::InputPort, elements))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::InputPort, totalElements))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::InputPort, totalMessages))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::InputPort, hasMessage))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::InputPort, labels))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::InputPort, removeLabel))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::InputPort, consume))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::InputPort, popMessage))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::InputPort, setReserve))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::InputPort, isSlot))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::InputPort, pushBuffer))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::InputPort, pushLabel))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::InputPort, pushMessage))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::InputPort, clear))
    .commit("Pothos/InputPort");
