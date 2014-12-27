// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include "Framework/PortSubscriber.hpp"
#include "Framework/WorkerActorMessages.hpp"
#include <Pothos/Framework/InputPort.hpp>
#include <Pothos/Framework/BufferAccumulator.hpp>
#include <Pothos/Util/RingDeque.hpp>
#include <Pothos/Util/SpinLock.hpp>
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
    void asyncMessagesPush(const TokenizedAsyncMessage &message);
    bool asyncMessagesEmpty(void);
    Pothos::Object asyncMessagesPop(void);
    void asyncMessagesClear(void);

    /////// inline message interface /////////
    std::vector<Label> inlineMessages;
    void inlineMessagesPush(const Label &label);
    void inlineMessagesMerge(void);
    void inlineMessagesClear(void);

    /////// input buffer interface /////////
    BufferChunk bufferAccumulatorFront;
    void bufferAccumulatorPush(const BufferChunk &buffer);
    void bufferAccumulatorPop(const size_t numBytes);
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
    Util::RingDeque<TokenizedAsyncMessage> _asyncMessages;

    /////// inline message storage /////////
    Util::SpinLock _inlineMessagesLock;
    Util::RingDeque<Label> _inputInlineMessages;

    /////// input buffer storage /////////
    Util::SpinLock _bufferAccumulatorLock;
    BufferAccumulator _bufferAccumulator;
};

inline void Pothos::InputPortImpl::asyncMessagesPush(const TokenizedAsyncMessage &message)
{
    std::unique_lock<Util::SpinLock> lock(_asyncMessagesLock);
    if (_asyncMessages.full()) _asyncMessages.set_capacity(_asyncMessages.capacity()*2);
    _asyncMessages.push_back(message);
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
    auto msg = _asyncMessages.front().async;
    _asyncMessages.pop_front();
    return msg;
}

inline void Pothos::InputPortImpl::asyncMessagesClear(void)
{
    std::unique_lock<Util::SpinLock> lock(_asyncMessagesLock);
    _asyncMessages.clear();
}

inline void Pothos::InputPortImpl::inlineMessagesPush(const Pothos::Label &label)
{
    std::unique_lock<Util::SpinLock> lock(_inlineMessagesLock);
    if (_inputInlineMessages.full()) _inputInlineMessages.set_capacity(_inputInlineMessages.capacity()*2);
    _inputInlineMessages.push_back(label);
}

inline void Pothos::InputPortImpl::inlineMessagesMerge(void)
{
    std::unique_lock<Util::SpinLock> lock(_inlineMessagesLock);
    //TODO
}

inline void Pothos::InputPortImpl::inlineMessagesClear(void)
{
    std::unique_lock<Util::SpinLock> lock(_inlineMessagesLock);
    _inputInlineMessages.clear();
    inlineMessages.clear();
}

inline void Pothos::InputPortImpl::bufferAccumulatorPush(const BufferChunk &buffer)
{
    std::unique_lock<Util::SpinLock> lock(_bufferAccumulatorLock);
    _bufferAccumulator.push(buffer);
    bufferAccumulatorFront = _bufferAccumulator.front();
}

inline void Pothos::InputPortImpl::bufferAccumulatorPop(const size_t numBytes)
{
    std::unique_lock<Util::SpinLock> lock(_bufferAccumulatorLock);
    _bufferAccumulator.pop(numBytes);
    bufferAccumulatorFront = _bufferAccumulator.front();
}

inline void Pothos::InputPortImpl::bufferAccumulatorRequire(const size_t numBytes)
{
    std::unique_lock<Util::SpinLock> lock(_bufferAccumulatorLock);
    _bufferAccumulator.require(numBytes);
    bufferAccumulatorFront = _bufferAccumulator.front();
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
    bufferAccumulatorFront = _bufferAccumulator.front();
}

inline void Pothos::InputPortImpl::bufferLabelPush(
    const Pothos::InputPort &input,
    const std::vector<Pothos::Label> &postedLabels,
    const Pothos::Util::RingDeque<Pothos::BufferChunk> &postedBuffers)
{
    std::unique_lock<Util::SpinLock> lock1(_inlineMessagesLock);
    std::unique_lock<Util::SpinLock> lock2(_bufferAccumulatorLock);

    const size_t requiredLabelSize = _inputInlineMessages.size() + postedLabels.size();
    if (_inputInlineMessages.capacity() < requiredLabelSize) _inputInlineMessages.set_capacity(requiredLabelSize);

    //insert labels (in order) and adjust for the current offset
    for (const auto &byteOffsetLabel : postedLabels)
    {
        auto label = byteOffsetLabel;
        auto elemSize = input.dtype().size();
        label.index += _bufferAccumulator.getTotalBytesAvailable(); //increment by enqueued bytes
        label.index /= elemSize; //convert from bytes to elements
        _inputInlineMessages.push_back(label);
    }

    //push all buffers into the accumulator
    for (size_t i = 0; i < postedBuffers.size(); i++)
    {
        _bufferAccumulator.push(postedBuffers[i]);
    }

    bufferAccumulatorFront = _bufferAccumulator.front();
}
