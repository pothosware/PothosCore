// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/InputPortImpl.hpp"

Pothos::InputPort::InputPort(InputPortImpl *impl):
    _impl(impl),
    _index(-1),
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
    return not _impl->asyncMessages.empty();
}

Pothos::Object Pothos::InputPort::popMessage(void)
{
    assert(_impl);
    if (_impl->asyncMessages.empty()) return Pothos::Object();
    auto msg = _impl->asyncMessages.front();
    _impl->asyncMessages.pop_front();
    _totalMessages++;
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
            return;
        }
    }
}

#include <Pothos/Managed.hpp>

static auto managedInputPort = Pothos::ManagedClass()
    .registerClass<Pothos::InputPort>()
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::InputPort, index))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::InputPort, name))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::InputPort, dtype))
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
    .commit("Pothos/InputPort");
