///
/// \file Framework/OutputPortImpl.hpp
///
/// Inline member implementation for OutputPort class.
///
/// \copyright
/// Copyright (c) 2014-2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Framework/OutputPort.hpp>
#include <mutex> //lock_guard

inline int Pothos::OutputPort::index(void) const
{
    return _index;
}

inline const std::string &Pothos::OutputPort::name(void) const
{
    return _name;
}

inline const Pothos::DType &Pothos::OutputPort::dtype(void) const
{
    return _dtype;
}

inline const std::string &Pothos::OutputPort::domain(void) const
{
    return _domain;
}

inline const Pothos::BufferChunk &Pothos::OutputPort::buffer(void) const
{
    return _buffer;
}

inline size_t Pothos::OutputPort::elements(void) const
{
    return _elements;
}

inline unsigned long long Pothos::OutputPort::totalElements(void) const
{
    return _totalElements;
}

inline unsigned long long Pothos::OutputPort::totalBuffers(void) const
{
    return _totalBuffers;
}

inline unsigned long long Pothos::OutputPort::totalLabels(void) const
{
    return _totalLabels;
}

inline unsigned long long Pothos::OutputPort::totalMessages(void) const
{
    return _totalMessages;
}

inline void Pothos::OutputPort::produce(const size_t numElements)
{
    _pendingElements += numElements;
}

inline bool Pothos::OutputPort::isSignal(void) const
{
    return _isSignal;
}

inline void Pothos::OutputPort::setReadBeforeWrite(InputPort *port)
{
    _readBeforeWritePort = port;
}

template <typename ValueType>
void Pothos::OutputPort::postMessage(ValueType &&message)
{
    Pothos::OutputPort::_postMessage(Pothos::Object(std::forward<ValueType>(message)));
}

inline void Pothos::OutputPort::popBuffer(const size_t numBytes)
{
    this->bufferManagerPop(numBytes);
    _workEvents++;
}

inline void Pothos::OutputPort::popElements(const size_t numElements)
{
    this->bufferManagerPop(numElements*this->dtype().size());
    _workEvents++;
}

inline void Pothos::OutputPort::postBuffer(const BufferChunk &buffer)
{
    auto &queue = _postedBuffers;
    if (queue.full()) queue.set_capacity(queue.size()*2);
    queue.push_back(buffer);

    //unspecified buffer dtype? copy it from the port
    if (not buffer.dtype) queue.back().dtype = this->dtype();

    _totalBuffers++;
    _workEvents++;
}

inline void Pothos::OutputPort::postLabel(const Label &label)
{
    _postedLabels.push_back(label.toAdjusted(this->dtype().size(), 1));
    _totalLabels++;
    _workEvents++;
}

inline bool Pothos::OutputPort::bufferManagerEmpty(void)
{
    std::lock_guard<Util::SpinLock> lock(_bufferManagerLock);
    return not _bufferManager or _bufferManager->empty();
}

inline void Pothos::OutputPort::bufferManagerFront(Pothos::BufferChunk &buff)
{
    std::lock_guard<Util::SpinLock> lock(_bufferManagerLock);
    if (not _bufferManager) buff = Pothos::BufferChunk();
    buff = _bufferManager->front();
}

inline void Pothos::OutputPort::bufferManagerPop(const size_t numBytes)
{
    std::lock_guard<Util::SpinLock> lock(_bufferManagerLock);
    return _bufferManager->pop(numBytes);
}

inline bool Pothos::OutputPort::tokenManagerEmpty(void)
{
    std::lock_guard<Util::SpinLock> lock(_tokenManagerLock);
    return not _tokenManager or _tokenManager->empty();
}

inline Pothos::BufferChunk Pothos::OutputPort::tokenManagerPop(void)
{
    std::lock_guard<Util::SpinLock> lock(_tokenManagerLock);
    if (_tokenManager->empty()) return Pothos::BufferChunk();
    auto tok = _tokenManager->front();
    _tokenManager->pop(0);
    return tok;
}

inline void Pothos::OutputPort::tokenManagerPop(const size_t numBytes)
{
    std::lock_guard<Util::SpinLock> lock(_tokenManagerLock);
    return _tokenManager->pop(numBytes);
}
