///
/// \file Framework/InputPortImpl.hpp
///
/// Inline member implementation for InputPort class.
///
/// \copyright
/// Copyright (c) 2014-2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Framework/InputPort.hpp>
#include <mutex> //lock_guard

inline int Pothos::InputPort::index(void) const
{
    return _index;
}

inline const std::string &Pothos::InputPort::name(void) const
{
    return _name;
}

inline const Pothos::DType &Pothos::InputPort::dtype(void) const
{
    return _dtype;
}

inline const std::string &Pothos::InputPort::domain(void) const
{
    return _domain;
}

inline const Pothos::BufferChunk &Pothos::InputPort::buffer(void) const
{
    return _buffer;
}

inline size_t Pothos::InputPort::elements(void) const
{
    return _elements;
}

inline unsigned long long Pothos::InputPort::totalElements(void) const
{
    return _totalElements;
}

inline unsigned long long Pothos::InputPort::totalBuffers(void) const
{
    return _totalBuffers;
}

inline unsigned long long Pothos::InputPort::totalLabels(void) const
{
    return _totalLabels;
}

inline unsigned long long Pothos::InputPort::totalMessages(void) const
{
    return _totalMessages;
}

inline const Pothos::LabelIteratorRange &Pothos::InputPort::labels(void) const
{
    return _labelIter;
}

inline void Pothos::InputPort::consume(const size_t numElements)
{
    _pendingElements += numElements;
}

inline bool Pothos::InputPort::hasMessage(void)
{
    return not this->asyncMessagesEmpty();
}

inline bool Pothos::InputPort::isSlot(void) const
{
    return _isSlot;
}

inline Pothos::Object Pothos::InputPort::popMessage(void)
{
    auto msg = this->asyncMessagesPop();
    _totalMessages++;
    _workEvents++;
    return msg;
}

inline Pothos::Object Pothos::InputPort::peekMessage(void)
{
    return this->asyncMessagesPeek();
}

inline void Pothos::InputPort::removeLabel(const Label &label)
{
    for (auto it = _inlineMessages.begin(); it != _inlineMessages.end(); it++)
    {
        if (*it == label)
        {
            _inlineMessages.erase(it);
            _labelIter = _inlineMessages;
            _totalLabels++;
            _workEvents++;
            return;
        }
    }
}

inline void Pothos::InputPort::setReserve(const size_t numElements)
{
    //only mark this change when setting a larger reserve
    if (numElements > _reserveElements) _workEvents++;

    _reserveElements = numElements;
}

inline bool Pothos::InputPort::asyncMessagesEmpty(void)
{
    std::lock_guard<Util::SpinLock> lock(_asyncMessagesLock);
    return _asyncMessages.empty();
}

inline Pothos::Object Pothos::InputPort::asyncMessagesPop(void)
{
    std::lock_guard<Util::SpinLock> lock(_asyncMessagesLock);
    if (_asyncMessages.empty()) return Pothos::Object();
    auto msg = std::move(_asyncMessages.front().first);
    _asyncMessages.pop_front();
    return msg;
}

inline Pothos::Object Pothos::InputPort::asyncMessagesPeek(void)
{
    std::lock_guard<Util::SpinLock> lock(_asyncMessagesLock);
    if (_asyncMessages.empty()) return Pothos::Object();
    return _asyncMessages.front().first;
}

inline void Pothos::InputPort::inlineMessagesPush(const Pothos::Label &label)
{
    std::lock_guard<Util::SpinLock> lock(_bufferAccumulatorLock);
    if (_inputInlineMessages.full()) _inputInlineMessages.set_capacity(_inputInlineMessages.capacity()*2);
    _inputInlineMessages.push_back(label);
}

inline void Pothos::InputPort::inlineMessagesClear(void)
{
    std::lock_guard<Util::SpinLock> lock(_bufferAccumulatorLock);
    _inputInlineMessages.clear();
    _inlineMessages.clear();
}

inline void Pothos::InputPort::bufferAccumulatorFront(Pothos::BufferChunk &buff)
{
    std::lock_guard<Util::SpinLock> lock(_bufferAccumulatorLock);
    while (not _inputInlineMessages.empty())
    {
        _inlineMessages.push_back(std::move(_inputInlineMessages.front()));
        _inlineMessages.back().adjust(1, this->dtype().size());
        _inputInlineMessages.pop_front();
    }
    buff = _bufferAccumulator.front();
}

inline void Pothos::InputPort::bufferAccumulatorPush(const BufferChunk &buffer)
{
    std::lock_guard<Util::SpinLock> lock(_bufferAccumulatorLock);
    this->bufferAccumulatorPushNoLock(BufferChunk(buffer));
}

inline void Pothos::InputPort::bufferAccumulatorRequire(const size_t numBytes)
{
    std::lock_guard<Util::SpinLock> lock(_bufferAccumulatorLock);
    _bufferAccumulator.require(numBytes);
}

inline void Pothos::InputPort::bufferAccumulatorClear(void)
{
    std::lock_guard<Util::SpinLock> lock(_bufferAccumulatorLock);
    _bufferAccumulator = BufferAccumulator();
}
