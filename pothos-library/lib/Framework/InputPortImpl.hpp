// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include "Framework/PortSubscriber.hpp"
#include <Pothos/Framework/InputPort.hpp>
#include <Pothos/Framework/BufferAccumulator.hpp>
#include <Pothos/Util/RingDeque.hpp>
#include <Pothos/Util/SpinLock.hpp>
#include <Pothos/Object/Containers.hpp>
#include <Poco/Logger.h>
#include <mutex> //unique_lock
#include <vector>

class Pothos::InputPortImpl
{
public:
    InputPortImpl(void):
        actor(nullptr),
        isSlot(false){}

    std::vector<PortSubscriber> subscribers;
    WorkerActor *actor;
    bool isSlot;

    /////// async message interface /////////
    void asyncMessagesPush(const Object &message, const BufferChunk &token = BufferChunk::null());
    bool asyncMessagesEmpty(void);
    Object asyncMessagesPop(void);
    void asyncMessagesClear(void);

    /////// slot call interface /////////
    void slotCallsPush(const ObjectVector &args, const BufferChunk &token);
    bool slotCallsEmpty(void);
    ObjectVector slotCallsPop(void);
    void slotCallsClear(void);

    /////// inline message interface /////////
    std::vector<Label> inlineMessages;
    void inlineMessagesPush(const Label &label);
    void inlineMessagesClear(void);

    /////// input buffer interface /////////
    BufferChunk bufferAccumulatorFront(void);
    void bufferAccumulatorPush(const InputPort &input, const BufferChunk &buffer);
    void bufferAccumulatorPushNoLock(const InputPort &input, const BufferChunk &buffer);
    void bufferAccumulatorPop(const size_t numBytes, const size_t numElems);
    void bufferAccumulatorRequire(const size_t numBytes);
    size_t bufferAccumulatorTotalBytes(void);
    void bufferAccumulatorClear(void);

    /////// combined label association push /////////
    void bufferLabelPush(
        const InputPort &port,
        const std::vector<Label> &postedLabels,
        const Util::RingDeque<BufferChunk> &postedBuffers);

private:
    /////// async message storage /////////
    Util::SpinLock _asyncMessagesLock;
    Util::RingDeque<std::pair<Object, BufferChunk>> _asyncMessages;

    /////// slot call storage /////////
    Util::SpinLock _slotCallsLock;
    Util::RingDeque<std::pair<ObjectVector, BufferChunk>> _slotCalls;

    /////// inline message storage /////////
    Util::RingDeque<Label> _inputInlineMessages;

    /////// input buffer storage /////////
    Util::SpinLock _bufferAccumulatorLock;
    BufferAccumulator _bufferAccumulator;
};

inline void Pothos::InputPortImpl::asyncMessagesPush(const Pothos::Object &message, const Pothos::BufferChunk &token)
{
    std::unique_lock<Util::SpinLock> lock(_asyncMessagesLock);
    if (_asyncMessages.full()) _asyncMessages.set_capacity(_asyncMessages.capacity()*2);
    _asyncMessages.push_back(std::make_pair(message, token));
}

inline bool Pothos::InputPortImpl::asyncMessagesEmpty(void)
{
    std::unique_lock<Util::SpinLock> lock(_asyncMessagesLock);
    return _asyncMessages.empty();
}

inline Pothos::Object Pothos::InputPortImpl::asyncMessagesPop(void)
{
    std::unique_lock<Util::SpinLock> lock(_asyncMessagesLock);
    if (_asyncMessages.empty()) return Pothos::Object();
    auto msg = _asyncMessages.front().first;
    _asyncMessages.pop_front();
    return msg;
}

inline void Pothos::InputPortImpl::asyncMessagesClear(void)
{
    std::unique_lock<Util::SpinLock> lock(_asyncMessagesLock);
    _asyncMessages.clear();
}

inline void Pothos::InputPortImpl::slotCallsPush(const Pothos::ObjectVector &args, const Pothos::BufferChunk &token)
{
    std::unique_lock<Util::SpinLock> lock(_slotCallsLock);
    if (_slotCalls.full()) _slotCalls.set_capacity(_slotCalls.capacity()*2);
    _slotCalls.push_back(std::make_pair(args, token));
}

inline bool Pothos::InputPortImpl::slotCallsEmpty(void)
{
    std::unique_lock<Util::SpinLock> lock(_slotCallsLock);
    return _slotCalls.empty();
}

inline Pothos::ObjectVector Pothos::InputPortImpl::slotCallsPop(void)
{
    std::unique_lock<Util::SpinLock> lock(_slotCallsLock);
    if (_slotCalls.empty()) return Pothos::ObjectVector();
    auto args = _slotCalls.front().first;
    _slotCalls.pop_front();
    return args;
}

inline void Pothos::InputPortImpl::slotCallsClear(void)
{
    std::unique_lock<Util::SpinLock> lock(_slotCallsLock);
    _slotCalls.clear();
}

inline void Pothos::InputPortImpl::inlineMessagesPush(const Pothos::Label &label)
{
    std::unique_lock<Util::SpinLock> lock(_bufferAccumulatorLock);
    if (_inputInlineMessages.full()) _inputInlineMessages.set_capacity(_inputInlineMessages.capacity()*2);
    _inputInlineMessages.push_back(label);
}

inline void Pothos::InputPortImpl::inlineMessagesClear(void)
{
    std::unique_lock<Util::SpinLock> lock(_bufferAccumulatorLock);
    _inputInlineMessages.clear();
    inlineMessages.clear();
}

inline Pothos::BufferChunk Pothos::InputPortImpl::bufferAccumulatorFront(void)
{
    std::unique_lock<Util::SpinLock> lock(_bufferAccumulatorLock);
    while (not _inputInlineMessages.empty())
    {
        inlineMessages.push_back(_inputInlineMessages.front());
        _inputInlineMessages.pop_front();
    }
    return _bufferAccumulator.front();
}

inline void Pothos::InputPortImpl::bufferAccumulatorPush(const Pothos::InputPort &input, const BufferChunk &buffer)
{
    std::unique_lock<Util::SpinLock> lock(_bufferAccumulatorLock);
    this->bufferAccumulatorPushNoLock(input, buffer);
}

inline void Pothos::InputPortImpl::bufferAccumulatorPushNoLock(const Pothos::InputPort &input, const BufferChunk &buffer_)
{
    auto buffer = buffer_;
    if (not buffer.dtype or not input.dtype() or //unspecified
        (input.dtype().size() == buffer.dtype.size())) //size match
    {
        //unspecified buffer dtype? copy it from the port
        if (not buffer.dtype) buffer.dtype = input.dtype();
        _bufferAccumulator.push(buffer);
    }
    else
    {
        poco_error_f4(Poco::Logger::get("Pothos.Block.inputBuffer"), "%s[%s] dropped '%s', expected '%s'",
            std::string("block->getName()")/*FIXME*/, input.name(), buffer.dtype.toString(), input.dtype().toString());
    }
}

inline void Pothos::InputPortImpl::bufferAccumulatorPop(const size_t numBytes, const size_t numElems)
{
    std::unique_lock<Util::SpinLock> lock(_bufferAccumulatorLock);
    _bufferAccumulator.pop(numBytes);

    //cant pop if labels enqueued, they will be wrong!
    for (size_t i = 0; i < _inputInlineMessages.size(); i++)
    {
        _inputInlineMessages[i].index -= numElems;
    }
}

inline void Pothos::InputPortImpl::bufferAccumulatorRequire(const size_t numBytes)
{
    std::unique_lock<Util::SpinLock> lock(_bufferAccumulatorLock);
    _bufferAccumulator.require(numBytes);
}

inline size_t Pothos::InputPortImpl::bufferAccumulatorTotalBytes(void)
{
    std::unique_lock<Util::SpinLock> lock(_bufferAccumulatorLock);
    return _bufferAccumulator.getTotalBytesAvailable();
}

inline void Pothos::InputPortImpl::bufferAccumulatorClear(void)
{
    std::unique_lock<Util::SpinLock> lock(_bufferAccumulatorLock);
    _bufferAccumulator = BufferAccumulator();
}

inline void Pothos::InputPortImpl::bufferLabelPush(
    const Pothos::InputPort &input,
    const std::vector<Pothos::Label> &postedLabels,
    const Pothos::Util::RingDeque<Pothos::BufferChunk> &postedBuffers)
{
    std::unique_lock<Util::SpinLock> lock1(_bufferAccumulatorLock);

    const size_t currentBytes = _bufferAccumulator.getTotalBytesAvailable();
    const size_t requiredLabelSize = _inputInlineMessages.size() + postedLabels.size();
    const size_t elemSize = input.dtype().size();
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
        this->bufferAccumulatorPushNoLock(input, postedBuffers[i]);
    }
}
