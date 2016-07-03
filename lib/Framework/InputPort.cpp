// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/InputPortImpl.hpp>
#include "Framework/WorkerActor.hpp"

/*!
 * An arbitrary bound on the queue size to detect buggy situations
 * where the downstream block is not consuming an input resource
 * and an upstream block is able to produce without back-pressure.
 * This should probably be a configurable system-wide value,
 * but for now the detection itself is more valuable to have.
 */
static const size_t MaxQueueCapacity = 1024;

Pothos::InputPort::InputPort(void):
    _actor(nullptr),
    _isSlot(false),
    _index(-1),
    _elements(0),
    _totalElements(0),
    _totalBuffers(0),
    _totalLabels(0),
    _totalMessages(0),
    _pendingElements(0),
    _reserveElements(0),
    _workEvents(0)
{
    return;
}

Pothos::InputPort::~InputPort(void)
{
    return;
}

const std::string &Pothos::InputPort::alias(void) const
{
    if (_alias.empty()) return this->name();
    return _alias;
}

void Pothos::InputPort::setAlias(const std::string &alias)
{
    _alias = alias;
}

void Pothos::InputPort::pushBuffer(const BufferChunk &buffer)
{
    this->bufferAccumulatorPush(buffer);
    assert(_actor != nullptr);
    _actor->flagExternalChange();
}

void Pothos::InputPort::pushLabel(const Label &label)
{
    this->inlineMessagesPush(label);
    assert(_actor != nullptr);
    _actor->flagExternalChange();
}

void Pothos::InputPort::pushMessage(const Object &message)
{
    this->asyncMessagesPush(message);
    //flagExternalChange() called in asyncMessagesPush...
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
    {
        std::lock_guard<Util::SpinLock> lock(_asyncMessagesLock);
        if (_asyncMessages.full())
        {
            if (_asyncMessages.size() >= MaxQueueCapacity)
            {
                _asyncMessages.clear();
                poco_error_f2(Poco::Logger::get("Pothos.InputPort.messages"),
                    "%s[%s] detected input message overflow condition",
                    _actor->block->getName(), this->alias());
            }
            else _asyncMessages.set_capacity(_asyncMessages.capacity()*2);
        }
        _asyncMessages.push_back(std::make_pair(message, token));
    }

    assert(_actor != nullptr);
    _actor->flagExternalChange();
}

void Pothos::InputPort::asyncMessagesClear(void)
{
    std::lock_guard<Util::SpinLock> lock(_asyncMessagesLock);
    _asyncMessages.clear();
}

void Pothos::InputPort::slotCallsPush(const Pothos::Object &args, const Pothos::BufferChunk &token)
{
    {
        std::lock_guard<Util::SpinLock> lock(_slotCallsLock);
        if (_slotCalls.full())
        {
            if (_slotCalls.size() >= MaxQueueCapacity)
            {
                _slotCalls.clear();
                poco_error_f2(Poco::Logger::get("Pothos.InputPort.slots"),
                    "%s[%s] detected input slot overflow condition",
                    _actor->block->getName(), this->alias());
            }
            else _slotCalls.set_capacity(_slotCalls.capacity()*2);
        }
        _slotCalls.push_back(std::make_pair(args, token));
    }

    assert(_actor != nullptr);
    _actor->flagExternalChange();
}

bool Pothos::InputPort::slotCallsEmpty(void)
{
    std::lock_guard<Util::SpinLock> lock(_slotCallsLock);
    return _slotCalls.empty();
}

Pothos::Object Pothos::InputPort::slotCallsPop(void)
{
    std::lock_guard<Util::SpinLock> lock(_slotCallsLock);
    assert(not _slotCalls.empty());
    auto args = _slotCalls.front().first;
    _slotCalls.pop_front();
    return args;
}

void Pothos::InputPort::slotCallsClear(void)
{
    std::lock_guard<Util::SpinLock> lock(_slotCallsLock);
    _slotCalls.clear();
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
        _totalBuffers++;
    }
    else
    {
        poco_error_f4(Poco::Logger::get("Pothos.Block.inputBuffer"), "%s[%s] dropped '%s', expected '%s'",
            _actor->block->getName(), this->alias(), buffer.dtype.toString(), this->dtype().toString());
    }
}

void Pothos::InputPort::bufferAccumulatorPop(const size_t numBytes)
{
    std::lock_guard<Util::SpinLock> lock(_bufferAccumulatorLock);

    if (numBytes > _bufferAccumulator.getTotalBytesAvailable())
    {
        poco_error_f4(Poco::Logger::get("Pothos.Block.consume"), "%s[%s] overconsumed %z bytes, %z available",
            _actor->block->getName(), this->alias(), numBytes, _bufferAccumulator.getTotalBytesAvailable());
        return;
    }

    _bufferAccumulator.pop(numBytes);

    //adjust enqueued inline messages for new offset
    for (size_t i = 0; i < _inputInlineMessages.size(); i++)
    {
        _inputInlineMessages[i].index -= numBytes;
    }

    _workEvents++;
}

void Pothos::InputPort::bufferLabelPush(
    const std::vector<Pothos::Label> &postedLabels,
    const Pothos::Util::RingDeque<Pothos::BufferChunk> &postedBuffers)
{
    {
        std::lock_guard<Util::SpinLock> lock(_bufferAccumulatorLock);

        const size_t currentBytes = _bufferAccumulator.getTotalBytesAvailable();
        const size_t requiredLabelSize = _inputInlineMessages.size() + postedLabels.size();
        if (_inputInlineMessages.capacity() < requiredLabelSize) _inputInlineMessages.set_capacity(requiredLabelSize);

        //insert labels (in order) and adjust for the current offset
        for (const auto &byteOffsetLabel : postedLabels)
        {
            auto label = byteOffsetLabel;
            label.index += currentBytes; //increment by enqueued bytes
            _inputInlineMessages.push_back(label);
        }

        //push all buffers into the accumulator
        for (size_t i = 0; i < postedBuffers.size(); i++)
        {
            this->bufferAccumulatorPushNoLock(postedBuffers[i]);
        }
    }

    assert(_actor != nullptr);
    _actor->flagExternalChange();
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
