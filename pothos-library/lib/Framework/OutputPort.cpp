// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/OutputPortImpl.hpp"
#include "Framework/WorkerActor.hpp"

void Pothos::OutputPortImpl::bufferManagerPush(Pothos::OutputPortImpl *self, const Pothos::ManagedBuffer &buff)
{
    auto mgr = buff.getBufferManager();
    if (mgr)
    {
        std::unique_lock<Util::SpinLock> lock(self->_bufferManagerLock);
        mgr->push(buff);
        self->actor->bump();
    }
}

void Pothos::OutputPortImpl::tokenManagerPush(Pothos::OutputPortImpl *self, const Pothos::ManagedBuffer &buff)
{
    auto mgr = buff.getBufferManager();
    if (mgr)
    {
        std::unique_lock<Util::SpinLock> lock(self->_tokenManagerLock);
        mgr->push(buff);
        self->actor->bump();
    }
}

Pothos::OutputPort::OutputPort(OutputPortImpl *impl):
    _impl(impl),
    _index(-1),
    _elements(0),
    _totalElements(0),
    _totalMessages(0),
    _pendingElements(0)
{
    return;
}

Pothos::OutputPort::~OutputPort(void)
{
    delete _impl;
}

void Pothos::OutputPort::popBuffer(const size_t numBytes)
{
    assert(_impl);
    _impl->bufferManagerPop(numBytes);
    _impl->actor->workBump = true;
}

void Pothos::OutputPort::postLabel(const Label &label)
{
    assert(_impl);
    assert(_impl->actor != nullptr);
    auto byteOffsetLabel = label;
    byteOffsetLabel.index *= this->dtype().size();
    _impl->postedLabels.push_back(byteOffsetLabel);
    _impl->actor->workBump = true;
}

void Pothos::OutputPort::_postMessage(const Object &async)
{
    assert(_impl);
    assert(_impl->actor != nullptr);
    TokenizedAsyncMessage message;
    message.token = _impl->tokenManagerPop();
    message.async = async;
    for (const auto &subscriber : _impl->subscribers)
    {
        subscriber.inputPort->_impl->asyncMessagesPush(message);
        subscriber.block->_actor->bump();
    }
    _totalMessages++;
    _impl->actor->workBump = true;
}

void Pothos::OutputPort::postBuffer(const BufferChunk &buffer)
{
    assert(_impl);
    auto &queue = _impl->postedBuffers;
    if (queue.full()) queue.set_capacity(queue.size()*2);
    queue.push_back(buffer);

    //unspecified buffer dtype? copy it from the port
    if (not buffer.dtype) queue.back().dtype = this->dtype();

    _impl->actor->workBump = true;
}

bool Pothos::OutputPort::isSignal(void) const
{
    assert(_impl);
    return _impl->isSignal;
}

void Pothos::OutputPort::setReadBeforeWrite(InputPort *port)
{
    assert(_impl);
    _impl->readBeforeWritePort = port;
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
