///
/// \file Framework/BufferAccumulator.hpp
///
/// BufferAccumulator provides an input pool of buffers.
///
/// \copyright
/// Copyright (c) 2013-2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Framework/BufferPool.hpp>
#include <Pothos/Framework/BufferChunk.hpp>
#include <Pothos/Util/RingDeque.hpp>
#include <cassert>

namespace Pothos {

/*!
 * The BufferAccumulator is a queue like interface
 * for holding and interacting with buffer chunks.
 *
 * A BufferAccumulator enqueus input buffers and presents
 * contiguous buffer chunks to the caller when applicable.
 * The BufferAccumulator may do several things:
 *  - Forward the same input buffers to the caller.
 *  - Amalgamate contiguous buffers into one.
 *  - Memcpy when the caller requires contiguity.
 */
class POTHOS_API BufferAccumulator
{
public:

    //! Create a new buffer accumulator
    BufferAccumulator(void);

    //! Clear the contents of this buffer accumulator
    void clear(void);

    /*!
     * Is the accumulator empty?
     * \return true when zero bytes available
     */
    bool empty(void) const;

    /*!
     * Push a buffer chunk into the accumulator.
     * \param buffer a buffer chunk
     */
    void push(const BufferChunk &buffer);

    /*!
     * Get the front buffer.
     * This reference is invalidated after mutator calls.
     * When empty, front returns an empty buffer.
     * \return a const reference to the front buffer
     */
    const BufferChunk &front(void) const;

    /*!
     * Pop numBytes from the front of this accumulator.
     * \param numBytes the number of bytes to remove
     */
    void pop(const size_t numBytes);

    //! Get the total number of bytes held in this accumulator
    size_t getTotalBytesAvailable(void) const;

    /*!
     * Enforce a requirement on the front buffer size.
     * If the front buffer is less than numBytes,
     * several buffers will be copied together
     * into a larger buffer with at least numBytes.
     * The effect of this call is over after pop().
     * \param numBytes the number of bytes needed
     */
    void require(const size_t numBytes);

    /*!
     * How many unique managed buffers are enqueued in this accumulator?
     * \warning expensive: this method is for debug/stats purposes.
     */
    size_t getUniqueManagedBufferCount(void) const;

private:
    Util::RingDeque<BufferChunk> _queue;
    size_t _bytesAvailable;
    bool _inPoolBuffer;
    BufferPool _pool;
};

} //namespace Pothos

inline bool Pothos::BufferAccumulator::empty(void) const
{
    return _bytesAvailable == 0;
}

inline const Pothos::BufferChunk &Pothos::BufferAccumulator::front(void) const
{
    assert(not _queue.empty());
    return _queue.front();
}

inline size_t Pothos::BufferAccumulator::getTotalBytesAvailable(void) const
{
    return _bytesAvailable;
}
