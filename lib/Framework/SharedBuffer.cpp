// Copyright (c) 2013-2017 Josh Blum
//                    2023 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include "MemoryMappedBufferContainer.hpp"

#include <Pothos/Framework/SharedBuffer.hpp>
#include <Pothos/Framework/Exception.hpp>
#include <algorithm> //min/max
#include <cassert>
#include <mutex>

/***********************************************************************
 * shared buffer implementation
 **********************************************************************/
const Pothos::SharedBuffer &Pothos::SharedBuffer::null(void)
{
    static SharedBuffer nullBuff;
    return nullBuff;
}

Pothos::SharedBuffer::SharedBuffer(void):
    _address(0), _length(0), _alias(0)
{
    return;
}

Pothos::SharedBuffer::SharedBuffer(const size_t address, const size_t length, std::shared_ptr<void> container):
    _address(address), _length(length), _alias(0), _container(container)
{
    return;
}

Pothos::SharedBuffer::SharedBuffer(const size_t address, const size_t length, const SharedBuffer &buffer):
    _address(address), _length(length), _alias(buffer._alias), _container(buffer._container)
{
    if (_alias != 0) _alias += _address - buffer.getAddress();
    const bool beginInRange = (_address >= buffer.getAddress())/* and (_address < buffer.getAddress() + buffer.getLength())*/;
    const bool endInRange = (_address + _length <= std::max(buffer.getAlias(), buffer.getAddress()) + buffer.getLength());
    if (not beginInRange or not endInRange)
    {
        throw SharedBufferError("Pothos::SharedBuffer()", "not a subset");
    }
}

/***********************************************************************
 * circular buffer implementation
 **********************************************************************/
static std::mutex &getCircMutex(void)
{
    static std::mutex mutex;
    return mutex;
}

Pothos::SharedBuffer Pothos::SharedBuffer::makeCirc(const size_t numBytes, const long nodeAffinity)
{
    //circular buffer implementations form a natural race condition
    //combine a mutex with retry logic to ensure the call succeeds
    const size_t numRetries = 7;
    for (size_t i = 0; i < numRetries; i++)
    {
        std::lock_guard<std::mutex> lock(getCircMutex());
        try
        {
            SharedBuffer buff = SharedBuffer::makeCircUnprotected(numBytes, nodeAffinity);
            buff._alias = buff.getAddress() + buff.getLength();
            return buff;
        }
        catch(const SharedBufferError &ex)
        {
            if (i == numRetries-1) throw ex;
        }
    }
    throw SharedBufferError("Pothos::SharedBuffer::makeCirc()", "invalid code path");
}

Pothos::SharedBuffer Pothos::SharedBuffer::makeFromFileMMap(const std::string &filepath, const bool readable, const bool writable)
{
    auto mmapContainer = MemoryMappedBufferContainer::make(filepath, readable, writable);
    assert(mmapContainer);
    assert(mmapContainer->buffer());
    assert(mmapContainer->length() > 0);

    return SharedBuffer(
        reinterpret_cast<size_t>(mmapContainer->buffer()),
        mmapContainer->length(),
        mmapContainer);
}
