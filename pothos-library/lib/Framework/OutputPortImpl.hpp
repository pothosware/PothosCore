// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include "Framework/PortSubscriber.hpp"
#include <Pothos/Framework/OutputPort.hpp>
#include <Pothos/Framework/BufferManager.hpp>
#include <Pothos/Util/RingDeque.hpp>
#include <Pothos/Util/SpinLock.hpp>
#include <mutex> //unique_lock
#include <vector>

class Pothos::OutputPortImpl
{
public:
    OutputPortImpl(void):
        actor(nullptr),
        isSignal(false),
        readBeforeWritePort(nullptr),
        _bufferFromManager(false)
    {
        this->tokenManagerInit();
    }

    /////// buffer manager /////////
    Util::SpinLock _bufferManagerLock;
    BufferManager::Sptr _bufferManager;
    void bufferManagerSetup(const BufferManager::Sptr &manager);
    bool bufferManagerEmpty(void);
    BufferChunk bufferManagerFront(void);
    void bufferManagerPop(const size_t numBytes);
    static void bufferManagerPush(Pothos::OutputPortImpl *self, const ManagedBuffer &buff);

    /////// token manager /////////
    Util::SpinLock _tokenManagerLock;
    BufferManager::Sptr _tokenManager; //used for message backpressure
    void tokenManagerInit(void);
    bool tokenManagerEmpty(void);
    BufferChunk tokenManagerPop(void);
    void tokenManagerPop(const size_t numBytes);
    static void tokenManagerPush(Pothos::OutputPortImpl *self, const ManagedBuffer &buff);

    std::vector<Label> postedLabels;
    Util::RingDeque<BufferChunk> postedBuffers;
    std::vector<PortSubscriber> subscribers;
    WorkerActor *actor;
    bool isSignal;
    InputPort *readBeforeWritePort;
    bool _bufferFromManager;
};

inline void Pothos::OutputPortImpl::bufferManagerSetup(const Pothos::BufferManager::Sptr &manager)
{
    std::unique_lock<Util::SpinLock> lock(_bufferManagerLock);
    _bufferManager = manager;
    if (manager) manager->setCallback(std::bind(
        &Pothos::OutputPortImpl::bufferManagerPush,
        this, std::placeholders::_1));
}

inline bool Pothos::OutputPortImpl::bufferManagerEmpty(void)
{
    std::unique_lock<Util::SpinLock> lock(_bufferManagerLock);
    return not _bufferManager or _bufferManager->empty();
}

inline Pothos::BufferChunk Pothos::OutputPortImpl::bufferManagerFront(void)
{
    std::unique_lock<Util::SpinLock> lock(_bufferManagerLock);
    return _bufferManager->front();
}

inline void Pothos::OutputPortImpl::bufferManagerPop(const size_t numBytes)
{
    std::unique_lock<Util::SpinLock> lock(_bufferManagerLock);
    return _bufferManager->pop(numBytes);
}

inline void Pothos::OutputPortImpl::bufferManagerPush(Pothos::OutputPortImpl *self, const Pothos::ManagedBuffer &buff)
{
    auto mgr = buff.getBufferManager();
    if (mgr)
    {
        std::unique_lock<Util::SpinLock> lock(self->_bufferManagerLock);
        mgr->push(buff);
        //TODO bump
    }
}

inline void Pothos::OutputPortImpl::tokenManagerInit(void)
{
    BufferManagerArgs tokenMgrArgs;
    tokenMgrArgs.numBuffers = 16;
    tokenMgrArgs.bufferSize = 0;
    _tokenManager = BufferManager::make("generic", tokenMgrArgs);
    _tokenManager->setCallback(std::bind(
        &Pothos::OutputPortImpl::tokenManagerPush,
        this, std::placeholders::_1));
}

inline bool Pothos::OutputPortImpl::tokenManagerEmpty(void)
{
    std::unique_lock<Util::SpinLock> lock(_tokenManagerLock);
    return _tokenManager->empty();
}

inline Pothos::BufferChunk Pothos::OutputPortImpl::tokenManagerPop(void)
{
    std::unique_lock<Util::SpinLock> lock(_tokenManagerLock);
    if (_tokenManager->empty()) return Pothos::BufferChunk();
    auto tok = _tokenManager->front();
    _tokenManager->pop(0);
    return tok;
}

inline void Pothos::OutputPortImpl::tokenManagerPop(const size_t numBytes)
{
    std::unique_lock<Util::SpinLock> lock(_tokenManagerLock);
    return _tokenManager->pop(numBytes);
}

inline void Pothos::OutputPortImpl::tokenManagerPush(Pothos::OutputPortImpl *self, const Pothos::ManagedBuffer &buff)
{
    auto mgr = buff.getBufferManager();
    if (mgr)
    {
        std::unique_lock<Util::SpinLock> lock(self->_tokenManagerLock);
        mgr->push(buff);
        //TODO bump
    }
}
