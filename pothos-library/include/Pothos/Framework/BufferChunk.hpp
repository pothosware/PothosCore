///
/// \file Framework/BufferChunk.hpp
///
/// A buffer chunk is a convenient interface to deal with
/// a managed or shared buffer and address/length offsets.
///
/// \copyright
/// Copyright (c) 2013-2014 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Framework/SharedBuffer.hpp>
#include <Pothos/Framework/ManagedBuffer.hpp>

namespace Pothos {

/*!
 * A BufferChunk holds a managed buffer, address, and length.
 * BufferChunk makes it easy to manipulate pieces of a managed buffer.
 * The caller manipulates a MangedBuffer into multiple BufferChunks;
 * then a BufferAccumulator can piece BufferChunks back together.
 */
class POTHOS_API BufferChunk
{
public:

    //! Get a const reference to a null/empty BufferChunk
    static const BufferChunk &null(void);

    /*!
     * Create a null BufferChunk.
     * The fields will be zeroed out and buffer null.
     */
    BufferChunk(void);

    /*!
     * Create a BufferChunk given the number of bytes.
     * This is a convenience method to create a BufferChunk
     * using memory from the standard memory allocator (new/delete).
     * Memory will automatically deallocate via reference counting.
     * \param numBytes the number of bytes to allocate
     */
    BufferChunk(const size_t numBytes);

    /*!
     * Create a BufferChunk from a SharedBuffer.
     * The fields will be initialized to that of the shared buffer.
     */
    BufferChunk(const SharedBuffer &buffer);

    /*!
     * Create a BufferChunk from a ManagedBuffer.
     * The fields will be initialized to that of the managed buffer.
     */
    BufferChunk(const ManagedBuffer &buffer);

    /*!
     * The address of the start of the buffer.
     */
    size_t address;

    /*!
     * The number of valid bytes in the buffer.
     */
    size_t length;

    /*!
     * The underlying reference counted shared buffer.
     */
    const SharedBuffer &getBuffer(void) const;

    /*!
     * The underlying reference counted managed buffer.
     */
    const ManagedBuffer &getManagedBuffer(void) const;

    /*!
     * Get the alias address of the address.
     * Only valid when the underlying buffer is circular.
     * \return the alias address or 0 if non-circular
     */
    size_t getAlias(void) const;

    /*!
     * Get the end address - front address + length.
     * The end address is non-inclusive.
     * \return the end address
     */
    size_t getEnd(void) const;

    /*!
     * Get a pointer to the front of the buffer
     * casted to the desired data type.
     */
    template <typename ElementType>
    ElementType as(void) const;

    /*!
     * Is the reference to the shared buffer unique?
     * \return true if this is the only copy of the shared buffer
     */
    bool unique(void) const;

    /*!
     * The number of copies of the shared buffer.
     * An extra copy held by the managed buffer is not counted.
     */
    size_t useCount(void) const;

    /*!
     * Is this buffer chunk valid?
     * \return true when there is underlying memory
     */
    pothos_explicit operator bool(void) const;

private:
    SharedBuffer _buffer;
    ManagedBuffer _managedBuffer;
};

} //namespace Pothos

#include <cassert>

inline const Pothos::SharedBuffer &Pothos::BufferChunk::getBuffer(void) const
{
    return _buffer;
}

inline const Pothos::ManagedBuffer &Pothos::BufferChunk::getManagedBuffer(void) const
{
    return _managedBuffer;
}

inline size_t Pothos::BufferChunk::getAlias(void) const
{
    if (_buffer.getAlias() == 0) return 0;
    size_t offset = _buffer.getAlias() - _buffer.getAddress();
    return address + offset;
}

inline size_t Pothos::BufferChunk::getEnd(void) const
{
    return address + length;
}

template <typename ElementType>
ElementType Pothos::BufferChunk::as(void) const
{
    return reinterpret_cast<ElementType>(address);
}

inline Pothos::BufferChunk::operator bool(void) const
{
    return (address != 0) or bool(_buffer);
}

inline bool Pothos::BufferChunk::unique(void) const
{
    return this->useCount() == 1;
}

inline size_t Pothos::BufferChunk::useCount(void) const
{
    //dont count the copy held by the managed buffer
    if (_managedBuffer)
    {
        assert(_buffer.useCount() >= 2);
        return _buffer.useCount() - 1;
    }
    return _buffer.useCount();
}
