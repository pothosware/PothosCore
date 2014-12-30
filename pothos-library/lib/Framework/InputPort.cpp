// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/InputPortImpl.hpp"
#include "Framework/WorkerActor.hpp"

Pothos::InputPort::InputPort(InputPortImpl *impl):
    _impl(impl),
    _index(-1),
    _buffer(BufferChunk::null()),
    _elements(0),
    _totalElements(0),
    _totalMessages(0),
    _pendingElements(0),
    _reserveElements(0)
{
    return;
}

Pothos::InputPort::~InputPort(void)
{
    delete _impl;
}

bool Pothos::InputPort::hasMessage(void) const
{
    assert(_impl);
    return not _impl->asyncMessagesEmpty();
}

Pothos::Object Pothos::InputPort::popMessage(void)
{
    assert(_impl);
    auto msg = _impl->asyncMessagesPop();
    _totalMessages++;
    _impl->actor->workBump = true;
    return msg;
}

void Pothos::InputPort::removeLabel(const Label &label)
{
    assert(_impl);
    for (auto it = _impl->inlineMessages.begin(); it != _impl->inlineMessages.end(); it++)
    {
        if (*it == label)
        {
            _impl->inlineMessages.erase(it);
            _labelIter = _impl->inlineMessages;
            _impl->actor->workBump = true;
            return;
        }
    }
}

void Pothos::InputPort::setReserve(const size_t numElements)
{
    _reserveElements = numElements;
    _impl->actor->workBump = true;
}

bool Pothos::InputPort::isSlot(void) const
{
    assert(_impl);
    return _impl->isSlot;
}

void Pothos::InputPort::pushBuffer(const BufferChunk &buffer)
{
    assert(_impl);
    _impl->bufferAccumulatorPush(*this, buffer);
    _impl->actor->flagChange();
}

void Pothos::InputPort::pushLabel(const Label &label)
{
    assert(_impl);
    _impl->inlineMessagesPush(label);
    _impl->actor->flagChange();
}

void Pothos::InputPort::pushMessage(const Object &message)
{
    assert(_impl);
    _impl->asyncMessagesPush(message);
    _impl->actor->flagChange();
}

void Pothos::InputPort::clear(void)
{
    assert(_impl);
    _impl->bufferAccumulatorClear();
    _impl->inlineMessagesClear();
    _impl->asyncMessagesClear();
    _impl->slotCallsClear();
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
