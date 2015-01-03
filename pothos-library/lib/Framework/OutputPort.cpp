// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/OutputPort.hpp>
#include "Framework/WorkerActor.hpp"

Pothos::OutputPort::OutputPort(void):
    _actor(nullptr),
    _index(-1),
    _elements(0),
    _totalElements(0),
    _totalMessages(0),
    _pendingElements(0),
    _isSignal(false),
    _readBeforeWritePort(nullptr),
    _bufferFromManager(false)
{
    return;
}

Pothos::OutputPort::~OutputPort(void)
{
    return;
}

void Pothos::OutputPort::popBuffer(const size_t numBytes)
{
    _bufferManager->pop(numBytes);
    _actor->workBump = true;
}

void Pothos::OutputPort::postLabel(const Label &label)
{
    assert(_actor != nullptr);
    auto byteOffsetLabel = label;
    byteOffsetLabel.index *= this->dtype().size();
    _postedLabels.push_back(byteOffsetLabel);
    _actor->workBump = true;
}

void Pothos::OutputPort::_postMessage(const Object &async)
{
    assert(_actor != nullptr);
    TokenizedAsyncMessage message;
    if (_tokenManager and not _tokenManager->empty())
    {
        message.token = _tokenManager->front();
        _tokenManager->pop(0);
        assert(message.token.unique());
    }
    message.async = async;
    _actor->sendOutputPortMessage(_subscribers, message);
    _totalMessages++;
    _actor->workBump = true;
}

void Pothos::OutputPort::postBuffer(const BufferChunk &buffer)
{
    auto &queue = _postedBuffers;
    if (queue.full()) queue.set_capacity(queue.size()*2);
    queue.push_back(buffer);

    //unspecified buffer dtype? copy it from the port
    if (not buffer.dtype) queue.back().dtype = this->dtype();

    _actor->workBump = true;
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
