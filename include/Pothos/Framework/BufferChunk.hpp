///
/// \file Framework/BufferChunk.hpp
///
/// A buffer chunk is a convenient interface to deal with
/// a managed or shared buffer and address/length offsets.
///
/// \copyright
/// Copyright (c) 2013-2017 Josh Blum
///                    2019 Nicholas Corgan
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Framework/DType.hpp>
#include <Pothos/Framework/SharedBuffer.hpp>
#include <Pothos/Framework/ManagedBuffer.hpp>
#include <utility> //move

namespace Pothos {

//! Forward declares
class BufferAccumulator;

/*!
 * A BufferChunk holds a managed buffer, address, and length.
 * BufferChunk makes it easy to manipulate pieces of a managed buffer.
 * The caller manipulates a ManagedBuffer into multiple BufferChunks;
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
     * Create a BufferChunk given data type and number of elements.
     * This is a convenience method to create a BufferChunk from a type.
     * using memory from the standard memory allocator (new/delete).
     * Memory will automatically deallocate via reference counting.
     * \param dtype the data type of the result buffer
     * \param numElems the size in number of elements
     */
    BufferChunk(const DType &dtype, const size_t numElems);

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

    //! BufferChunk copy constructor
    BufferChunk(const BufferChunk &other);

    //! BufferChunk move constructor
    BufferChunk(BufferChunk &&other) noexcept;

    //! BufferChunk destructor
    ~BufferChunk(void);

    //! BufferChunk copy assignment operator
    BufferChunk &operator=(const BufferChunk &other);

    //! BufferChunk move assignment operator
    BufferChunk &operator=(BufferChunk &&other);

    /*!
     * The address of the start of the buffer.
     */
    size_t address;

    /*!
     * The number of valid bytes in the buffer.
     */
    size_t length;

    /*!
     * The data type of the contents of this buffer.
     */
    DType dtype;

    /*!
     * How many elements are held in this buffer chunk?
     * \return the length in bytes divided by the dtype size.
     */
    size_t elements(void) const;

    /*!
     * Set the number of elements held by this buffer.
     * This call modifies the length field in bytes based on the
     * configured data type and the specified number of elements.
     */
    void setElements(const size_t numElements);

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
     * Get a pointer to the front of the buffer as the specified data type.
     * \code
     * auto ptr = buffer.as<const int *>();
     * \endcode
     * \tparam ElementType the desired pointer type
     * \return the front of the buffer casted into the specified pointer type
     */
    template <typename ElementType>
    ElementType as(void) const;

    /*!
     * Get a pointer to the front of the buffer as the specified data type.
     * This call overloads the conversion operator to provide implicit conversion.
     * \code
     * const int *ptr = buffer;
     * \endcode
     * \return the front of the buffer casted into the specified pointer type
     * \tparam ElementType the desired pointer type
     */
    template <typename ElementType>
    operator ElementType(void) const;

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
    explicit operator bool(void) const;

    /*!
     * Make this buffer chunk null by clearing its contents.
     * All fields will be zero and containers will be empty.
     */
    void clear(void);

    //! Serialization support
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);

    /*!
     * Append another buffer onto the back of this buffer.
     * This call allocates a new memory slab the size of both buffers
     * and copies the contents from each one into the new memory slab.
     * The length and address members will be updated accordingly.
     * When empty, append simply copies a reference to the other buffer.
     * \param other the other buffer to append to the end
     */
    void append(const BufferChunk &other);

    /*!
     * Convert a buffer chunk to the specified data type.
     * When the number of elements are 0, the entire buffer is converted.
     * \throws BufferConvertError when the conversion is not possible
     * \param dtype the data type of the result buffer
     * \param numElems the number of elements to convert
     * \return a new buffer chunk with converted elements
     */
    BufferChunk convert(const DType &dtype, const size_t numElems = 0) const;

    /*!
     * Convert a buffer chunk of complex elements to two real buffers.
     * When the number of elements are 0, the entire buffer is converted.
     * \throws BufferConvertError when the conversion is not possible
     * \param dtype the data type of the result buffer
     * \param numElems the number of elements to convert
     * \return a real + complex pair of buffer chunks
     */
    std::pair<BufferChunk, BufferChunk> convertComplex(const DType &dtype, const size_t numElems = 0) const;

    /*!
     * Convert a buffer chunk into the specified output buffer.
     * When the number of elements are 0, the entire buffer is converted.
     * The buffer length should be large enough to contain the entire conversion.
     * \throws BufferConvertError when the conversion is not possible
     * \param [out] outBuff the output buffer, also specifies the dtype
     * \param numElems the number of elements to convert
     * \return the number of output elements written to the buffer
     */
    size_t convert(const BufferChunk &outBuff, const size_t numElems = 0) const;

    /*!
     * Convert a buffer chunk of complex elements into two real buffers.
     * When the number of elements are 0, the entire buffer is converted.
     * The buffer length should be large enough to contain the entire conversion.
     * \throws BufferConvertError when the conversion is not possible
     * \param [out] outBuffRe the real output buffer, also specifies the dtype
     * \param [out] outBuffIm the imaginary output buffer, also specifies the dtype
     * \param numElems the number of elements to convert
     * \return the number of output elements written to the buffers
     */
    size_t convertComplex(const BufferChunk &outBuffRe, const BufferChunk &outBuffIm, const size_t numElems = 0) const;

private:
    friend BufferAccumulator;
    ManagedBuffer _managedBuffer;
    void _incrNextBuffers(void);
    void _decrNextBuffers(void);
    size_t _nextBuffers;
};

/*!
 * Equality operator for BufferChunk objects.
 * True when the address, length, and underlying buffer are the same.
 */
inline bool operator==(const BufferChunk &lhs, const BufferChunk &rhs);

/*!
 * Inquality operator for BufferChunk objects.
 * True when the address, length, or underlying buffer are different.
 */
inline bool operator!=(const BufferChunk &lhs, const BufferChunk &rhs);

} //namespace Pothos

inline bool Pothos::operator==(const Pothos::BufferChunk &lhs, const Pothos::BufferChunk &rhs)
{
    return lhs.address == rhs.address and lhs.length == rhs.length and lhs.getBuffer() == rhs.getBuffer();
}

inline bool Pothos::operator!=(const Pothos::BufferChunk &lhs, const Pothos::BufferChunk &rhs)
{
    return !(lhs == rhs);
}

inline Pothos::BufferChunk::BufferChunk(void):
    address(0),
    length(0),
    _nextBuffers(0)
{
    return;
}

inline Pothos::BufferChunk::BufferChunk(const size_t numBytes):
    address(0),
    length(numBytes),
    _managedBuffer(Pothos::SharedBuffer::make(numBytes)),
    _nextBuffers(0)
{
    address = getBuffer().getAddress();
}

inline Pothos::BufferChunk::BufferChunk(const DType &dtype, const size_t numElems):
    address(0),
    length(dtype.size()*numElems),
    dtype(dtype),
    _managedBuffer(Pothos::SharedBuffer::make(length)),
    _nextBuffers(0)
{
    address = getBuffer().getAddress();
}

inline Pothos::BufferChunk::BufferChunk(const SharedBuffer &buffer):
    address(buffer.getAddress()),
    length(buffer.getLength()),
    _managedBuffer(buffer),
    _nextBuffers(0)
{
    return;
}

inline Pothos::BufferChunk::BufferChunk(const ManagedBuffer &buffer):
    address(buffer.getBuffer().getAddress()),
    length(buffer.getBuffer().getLength()),
    _managedBuffer(buffer),
    _nextBuffers(0)
{
    return;
}

inline Pothos::BufferChunk::BufferChunk(const BufferChunk &other):
    address(other.address),
    length(other.length),
    dtype(other.dtype),
    _managedBuffer(other._managedBuffer)
{
    _incrNextBuffers();
}

inline Pothos::BufferChunk::BufferChunk(BufferChunk &&other) noexcept:
    address(std::move(other.address)),
    length(std::move(other.length)),
    dtype(std::move(other.dtype)),
    _managedBuffer(std::move(other._managedBuffer)),
    _nextBuffers(std::move(other._nextBuffers))
{
    other.address = 0;
    other.length = 0;
    other._nextBuffers = 0;
}

inline Pothos::BufferChunk::~BufferChunk(void)
{
    _decrNextBuffers();
}

inline Pothos::BufferChunk &Pothos::BufferChunk::operator=(const BufferChunk &other)
{
    _decrNextBuffers();
    address = other.address;
    length = other.length;
    dtype = other.dtype;
    _managedBuffer = other._managedBuffer;
    _incrNextBuffers();
    return *this;
}

inline Pothos::BufferChunk &Pothos::BufferChunk::operator=(BufferChunk &&other)
{
    _decrNextBuffers();
    address = std::move(other.address);
    length = std::move(other.length);
    dtype = std::move(other.dtype);
    _managedBuffer = std::move(other._managedBuffer);
    _nextBuffers = std::move(other._nextBuffers);
    other.address = 0;
    other.length = 0;
    other._nextBuffers = 0;
    return *this;
}

inline void Pothos::BufferChunk::clear(void)
{
    _decrNextBuffers();
    address = 0;
    length = 0;
    dtype = Pothos::DType();
    _managedBuffer.reset();
    _nextBuffers = 0;
}

inline size_t Pothos::BufferChunk::elements(void) const
{
    return this->length/this->dtype.size();
}

inline void Pothos::BufferChunk::setElements(const size_t numElements)
{
    this->length = numElements*this->dtype.size();
}

inline const Pothos::SharedBuffer &Pothos::BufferChunk::getBuffer(void) const
{
    return _managedBuffer.getBuffer();
}

inline const Pothos::ManagedBuffer &Pothos::BufferChunk::getManagedBuffer(void) const
{
    return _managedBuffer;
}

inline size_t Pothos::BufferChunk::getAlias(void) const
{
    const auto &buffer = getBuffer();
    if (buffer.getAlias() == 0) return 0;
    if (address > buffer.getAlias()) return address - buffer.getLength();
    else return address + buffer.getLength();
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

template <typename ElementType>
Pothos::BufferChunk::operator ElementType(void) const
{
    return reinterpret_cast<ElementType>(address);
}

inline Pothos::BufferChunk::operator bool(void) const
{
    return bool(_managedBuffer);
}

inline bool Pothos::BufferChunk::unique(void) const
{
    return _managedBuffer.unique();
}

inline size_t Pothos::BufferChunk::useCount(void) const
{
    return _managedBuffer.useCount();
}
