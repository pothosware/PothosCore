// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/OutputPortImpl.hpp>
#include "Framework/WorkerActor.hpp"
#include <Pothos/Object/Containers.hpp>

Pothos::OutputPort::OutputPort(void):
    _actor(nullptr),
    _isSignal(false),
    _index(-1),
    _elements(0),
    _totalElements(0),
    _totalBuffers(0),
    _totalLabels(0),
    _totalMessages(0),
    _pendingElements(0),
    _workEvents(0),
    _readBeforeWritePort(nullptr),
    _bufferFromManager(false)
{
    this->tokenManagerInit();
}

Pothos::OutputPort::~OutputPort(void)
{
    return;
}

const std::string &Pothos::OutputPort::alias(void) const
{
    if (_alias.empty()) return this->name();
    return _alias;
}

void Pothos::OutputPort::setAlias(const std::string &alias)
{
    _alias = alias;
}

void Pothos::OutputPort::_postMessage(const Object &async)
{
    const auto token = this->tokenManagerPop();
    for (const auto &subscriber : _subscribers)
    {
        if (subscriber->isSlot() and async.type() == typeid(ObjectVector))
        {
            subscriber->slotCallsPush(async, token);
        }
        else
        {
            subscriber->asyncMessagesPush(async, token);
        }
    }
    _totalMessages++;
    _workEvents++;
}

void Pothos::OutputPort::bufferManagerPush(Pothos::Util::SpinLock *mutex, const Pothos::ManagedBuffer &buff)
{
    auto mgr = buff.getBufferManager();
    if (mgr)
    {
        std::lock_guard<Pothos::Util::SpinLock> lock(*mutex);
        mgr->push(buff);
        assert(_actor != nullptr);
        _actor->flagExternalChange();
    }
}

void Pothos::OutputPort::bufferManagerSetup(const Pothos::BufferManager::Sptr &manager)
{
    std::lock_guard<Util::SpinLock> lock(_bufferManagerLock);
    _bufferManager = manager;
    if (manager) manager->setCallback(std::bind(
        &Pothos::OutputPort::bufferManagerPush, this, &_bufferManagerLock, std::placeholders::_1));
}

void Pothos::OutputPort::tokenManagerInit(void)
{
    BufferManagerArgs tokenMgrArgs;
    tokenMgrArgs.numBuffers = 16;
    tokenMgrArgs.bufferSize = 0;
    _tokenManager = BufferManager::make("generic", tokenMgrArgs);
    _tokenManager->setCallback(std::bind(
        &Pothos::OutputPort::bufferManagerPush, this, &_tokenManagerLock, std::placeholders::_1));
}

#include <Pothos/Managed.hpp>

static auto managedOutputPort = Pothos::ManagedClass()
    .registerClass<Pothos::OutputPort>()
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::OutputPort, index))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::OutputPort, name))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::OutputPort, dtype))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::OutputPort, domain))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::OutputPort, buffer))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::OutputPort, elements))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::OutputPort, totalElements))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::OutputPort, totalMessages))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::OutputPort, produce))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::OutputPort, popBuffer))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::OutputPort, postLabel))
    .registerMethod<const Pothos::Object &, void, Pothos::OutputPort>(POTHOS_FCN_TUPLE(Pothos::OutputPort, postMessage))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::OutputPort, postBuffer))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::OutputPort, isSignal))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::OutputPort, setReadBeforeWrite))
    .commit("Pothos/OutputPort");
