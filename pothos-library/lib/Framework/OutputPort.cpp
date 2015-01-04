// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/OutputPort.hpp>
#include "Framework/WorkerActor.hpp"

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

void Pothos::OutputPort::popBuffer(const size_t numBytes)
{
    this->bufferManagerPop(numBytes);
    _workEvents++;
}

void Pothos::OutputPort::postLabel(const Label &label)
{
    auto byteOffsetLabel = label;
    byteOffsetLabel.index *= this->dtype().size();
    _postedLabels.push_back(byteOffsetLabel);
    _totalLabels++;
    _workEvents++;
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
        subscriber->_actor->flagChange();
    }
    _totalMessages++;
    _workEvents++;
}

void Pothos::OutputPort::postBuffer(const BufferChunk &buffer)
{
    auto &queue = _postedBuffers;
    if (queue.full()) queue.set_capacity(queue.size()*2);
    queue.push_back(buffer);

    //unspecified buffer dtype? copy it from the port
    if (not buffer.dtype) queue.back().dtype = this->dtype();

    _totalBuffers++;
    _workEvents++;
}

void Pothos::OutputPort::bufferManagerPush(Pothos::Util::SpinLock *mutex, const Pothos::ManagedBuffer &buff)
{
    auto mgr = buff.getBufferManager();
    if (mgr)
    {
        std::unique_lock<Pothos::Util::SpinLock> lock(*mutex);
        mgr->push(buff);
        assert(_actor != nullptr);
        _actor->flagChange();
    }
}

void Pothos::OutputPort::bufferManagerSetup(const Pothos::BufferManager::Sptr &manager)
{
    std::unique_lock<Util::SpinLock> lock(_bufferManagerLock);
    _bufferManager = manager;
    if (manager) manager->setCallback(std::bind(
        &Pothos::OutputPort::bufferManagerPush, this, &_bufferManagerLock, std::placeholders::_1));
}

bool Pothos::OutputPort::bufferManagerEmpty(void)
{
    std::unique_lock<Util::SpinLock> lock(_bufferManagerLock);
    return not _bufferManager or _bufferManager->empty();
}

Pothos::BufferChunk Pothos::OutputPort::bufferManagerFront(void)
{
    std::unique_lock<Util::SpinLock> lock(_bufferManagerLock);
    return _bufferManager->front();
}

void Pothos::OutputPort::bufferManagerPop(const size_t numBytes)
{
    std::unique_lock<Util::SpinLock> lock(_bufferManagerLock);
    return _bufferManager->pop(numBytes);
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

bool Pothos::OutputPort::tokenManagerEmpty(void)
{
    std::unique_lock<Util::SpinLock> lock(_tokenManagerLock);
    return _tokenManager->empty();
}

Pothos::BufferChunk Pothos::OutputPort::tokenManagerPop(void)
{
    std::unique_lock<Util::SpinLock> lock(_tokenManagerLock);
    if (_tokenManager->empty()) return Pothos::BufferChunk();
    auto tok = _tokenManager->front();
    _tokenManager->pop(0);
    return tok;
}

void Pothos::OutputPort::tokenManagerPop(const size_t numBytes)
{
    std::unique_lock<Util::SpinLock> lock(_tokenManagerLock);
    return _tokenManager->pop(numBytes);
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
