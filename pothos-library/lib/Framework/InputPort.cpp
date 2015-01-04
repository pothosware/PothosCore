// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/InputPort.hpp>
#include "Framework/WorkerActor.hpp"

Pothos::InputPort::InputPort(void):
    _actor(nullptr),
    _index(-1),
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
    auto msg = this->asyncMessagesPop();
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
    this->bufferAccumulatorPush(buffer);
    _actor->flagChange();
}

void Pothos::InputPort::pushLabel(const Label &label)
{
    this->inlineMessagesPush(label);
    _actor->flagChange();
}

void Pothos::InputPort::pushMessage(const Object &message)
{
    this->asyncMessagesPush(message);
    _actor->flagChange();
}

void Pothos::InputPort::clear(void)
{
    this->bufferAccumulatorClear();
    this->inlineMessagesClear();
    this->asyncMessagesClear();
    this->slotCallsClear();
}

void Pothos::InputPort::asyncMessagesPush(const Pothos::Object &message, const Pothos::BufferChunk &token)
{
    std::unique_lock<Util::SpinLock> lock(_asyncMessagesLock);
    if (_asyncMessages.full()) _asyncMessages.set_capacity(_asyncMessages.capacity()*2);
    _asyncMessages.push_back(std::make_pair(message, token));
}

bool Pothos::InputPort::asyncMessagesEmpty(void)
{
    std::unique_lock<Util::SpinLock> lock(_asyncMessagesLock);
    return _asyncMessages.empty();
}

Pothos::Object Pothos::InputPort::asyncMessagesPop(void)
{
    std::unique_lock<Util::SpinLock> lock(_asyncMessagesLock);
    if (_asyncMessages.empty()) return Pothos::Object();
    auto msg = _asyncMessages.front().first;
    _asyncMessages.pop_front();
    return msg;
}

void Pothos::InputPort::asyncMessagesClear(void)
{
    std::unique_lock<Util::SpinLock> lock(_asyncMessagesLock);
    _asyncMessages.clear();
}

void Pothos::InputPort::slotCallsPush(const Pothos::Object &args, const Pothos::BufferChunk &token)
{
    std::unique_lock<Util::SpinLock> lock(_slotCallsLock);
    if (_slotCalls.full()) _slotCalls.set_capacity(_slotCalls.capacity()*2);
    _slotCalls.push_back(std::make_pair(args, token));
}

bool Pothos::InputPort::slotCallsEmpty(void)
{
    std::unique_lock<Util::SpinLock> lock(_slotCallsLock);
    return _slotCalls.empty();
}

Pothos::Object Pothos::InputPort::slotCallsPop(void)
{
    std::unique_lock<Util::SpinLock> lock(_slotCallsLock);
    if (_slotCalls.empty()) return Pothos::Object();
    auto args = _slotCalls.front().first;
    _slotCalls.pop_front();
    return args;
}

void Pothos::InputPort::slotCallsClear(void)
{
    std::unique_lock<Util::SpinLock> lock(_slotCallsLock);
    _slotCalls.clear();
}

void Pothos::InputPort::inlineMessagesPush(const Pothos::Label &label)
{
    std::unique_lock<Util::SpinLock> lock(_bufferAccumulatorLock);
    if (_inputInlineMessages.full()) _inputInlineMessages.set_capacity(_inputInlineMessages.capacity()*2);
    _inputInlineMessages.push_back(label);
}

void Pothos::InputPort::inlineMessagesClear(void)
{
    std::unique_lock<Util::SpinLock> lock(_bufferAccumulatorLock);
    _inputInlineMessages.clear();
    _inlineMessages.clear();
}

Pothos::BufferChunk Pothos::InputPort::bufferAccumulatorFront(void)
{
    std::unique_lock<Util::SpinLock> lock(_bufferAccumulatorLock);
    while (not _inputInlineMessages.empty())
    {
        _inlineMessages.push_back(_inputInlineMessages.front());
        _inputInlineMessages.pop_front();
    }
    return _bufferAccumulator.front();
}

void Pothos::InputPort::bufferAccumulatorPush(const BufferChunk &buffer)
{
    std::unique_lock<Util::SpinLock> lock(_bufferAccumulatorLock);
    this->bufferAccumulatorPushNoLock(buffer);
}

void Pothos::InputPort::bufferAccumulatorPushNoLock(const BufferChunk &buffer_)
{
    auto buffer = buffer_;
    if (not buffer.dtype or not this->dtype() or //unspecified
        (this->dtype().size() == buffer.dtype.size())) //size match
    {
        //unspecified buffer dtype? copy it from the port
        if (not buffer.dtype) buffer.dtype = this->dtype();
        _bufferAccumulator.push(buffer);
    }
    else
    {
        poco_error_f4(Poco::Logger::get("Pothos.Block.inputBuffer"), "%s[%s] dropped '%s', expected '%s'",
            _actor->block->getName(), this->name(), buffer.dtype.toString(), this->dtype().toString());
    }
}

void Pothos::InputPort::bufferAccumulatorPop(const size_t numBytes, const size_t numElems)
{
    std::unique_lock<Util::SpinLock> lock(_bufferAccumulatorLock);
    _bufferAccumulator.pop(numBytes);

    //cant pop if labels enqueued, they will be wrong!
    for (size_t i = 0; i < _inputInlineMessages.size(); i++)
    {
        _inputInlineMessages[i].index -= numElems;
    }
}

void Pothos::InputPort::bufferAccumulatorRequire(const size_t numBytes)
{
    std::unique_lock<Util::SpinLock> lock(_bufferAccumulatorLock);
    _bufferAccumulator.require(numBytes);
}

size_t Pothos::InputPort::bufferAccumulatorTotalBytes(void)
{
    std::unique_lock<Util::SpinLock> lock(_bufferAccumulatorLock);
    return _bufferAccumulator.getTotalBytesAvailable();
}

void Pothos::InputPort::bufferAccumulatorClear(void)
{
    std::unique_lock<Util::SpinLock> lock(_bufferAccumulatorLock);
    _bufferAccumulator = BufferAccumulator();
}

void Pothos::InputPort::bufferLabelPush(
    const std::vector<Pothos::Label> &postedLabels,
    const Pothos::Util::RingDeque<Pothos::BufferChunk> &postedBuffers)
{
    std::unique_lock<Util::SpinLock> lock1(_bufferAccumulatorLock);

    const size_t currentBytes = _bufferAccumulator.getTotalBytesAvailable();
    const size_t requiredLabelSize = _inputInlineMessages.size() + postedLabels.size();
    const size_t elemSize = this->dtype().size();
    if (_inputInlineMessages.capacity() < requiredLabelSize) _inputInlineMessages.set_capacity(requiredLabelSize);

    //insert labels (in order) and adjust for the current offset
    for (const auto &byteOffsetLabel : postedLabels)
    {
        auto label = byteOffsetLabel;
        label.index += currentBytes; //increment by enqueued bytes
        label.index /= elemSize; //convert from bytes to elements
        _inputInlineMessages.push_back(label);
    }

    //push all buffers into the accumulator
    for (size_t i = 0; i < postedBuffers.size(); i++)
    {
        this->bufferAccumulatorPushNoLock(postedBuffers[i]);
    }
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
