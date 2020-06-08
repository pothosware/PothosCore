///
/// \file Framework/SharedBuffer.hpp
///
/// The shared buffer is an RAII buffer that automatically deallocates.
///
/// \copyright
/// Copyright (c) 2013-2017 Josh Blum
///                    2019 Nicholas Corgan
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <memory> //shared_ptr

namespace Pothos {

/*!
 * The SharedBuffer represents the combination of address and length,
 * and a shared pointer that will automatically cleanup the memory.
 */
class POTHOS_API SharedBuffer
{
public:

    //! Get a const reference to a null/empty SharedBuffer
    static const SharedBuffer &null(void);

    //! Create a null SharedBuffer
    SharedBuffer(void);

    /*!
     * Create a SharedBuffer given a length in bytes.
     * This factory allocates memory which is held by the SharedBuffer.
     * When the SharedBuffer is deleted, the memory will be freed as well.
     * The node affinity is used to allocate physical memory on a NUMA node.
     *
     * \param numBytes the number of bytes to allocate in this buffer
     * \param nodeAffinity which NUMA node to allocate on (-1 for dont care)
     * \return a new shared buffer object
     */
    static SharedBuffer make(const size_t numBytes, const long nodeAffinity = -1);

    /*!
     * Create a circular SharedBuffer given a length in bytes.
     * The rules for the circular or double mapping are as follows:
     * for i in 0 to length-1: address + i == address + i + length
     *
     * This factory allocates memory which is held by the SharedBuffer.
     * When the SharedBuffer is deleted, the memory will be freed as well.
     * The node affinity is used to allocate physical memory on a NUMA node.
     *
     * \param numBytes the number of bytes to allocate in this buffer
     * \param nodeAffinity which NUMA node to allocate on (-1 for dont care)
     * \return a new circular shared buffer object
     */
    static SharedBuffer makeCirc(const size_t numBytes, const long nodeAffinity = -1);

    /*!
     * Create a SharedBuffer from address, length, and the container.
     * The container is any object that can be put into a shared_ptr.
     * When the shared pointer is deleted, the container's destructor
     * (written by the user) should handle the cleanup of the memory.
     */
    SharedBuffer(const size_t address, const size_t length, std::shared_ptr<void> container);

    /*!
     * Create a sub-buffer that is a subset of the buffer.
     * The shared container is copied so the sub-buffer holds a reference.
     * \throws SharedBufferError if the parameters would be out of bounds
     * \param address an address within the buffer
     * \param length the new length of the sub buffer
     * \param buffer a shared buffer which is a superset of this new one
     */
    SharedBuffer(const size_t address, const size_t length, const SharedBuffer &buffer);

    //! Get the address of the first byte of the buffer
    size_t getAddress(void) const;

    //! Get the length of the buffer in bytes
    size_t getLength(void) const;

    /*!
     * Get the alias address (non-zero for circular buffers).
     * Address and alias will point to the same physical memory.
     */
    size_t getAlias(void) const;

    /*!
     * Get the alias offset (non-zero for circular buffers).
     * Address and alias will point to the same physical memory.
     */
    size_t getAliasOffset(void) const;

    /*!
     * Set the alias address (non-zero for circular buffers).
     * Address and alias will point to the same physical memory.
     */
    void setAlias(const size_t alias);

    /*!
     * Get the end address - front address + length.
     * The end address is non-inclusive.
     * \return the end address
     */
    size_t getEnd(void) const;

    /*!
     * Is this instance of this shared buffer unique?
     * \return true if this is the only copy
     */
    bool unique(void) const;

    /*!
     * The number of copies of this shared buffer.
     */
    size_t useCount(void) const;

    /*!
     * Is this shared buffer valid?
     */
    explicit operator bool(void) const;

    /*!
     * Get access to the underlying memory container.
     */
    const std::shared_ptr<void> &getContainer(void) const;

private:
    static SharedBuffer makeCircUnprotected(const size_t numBytes, const long nodeAffinity);
    size_t _address;
    size_t _length;
    size_t _alias;
    std::shared_ptr<void> _container;
};

/*!
 * Equality operator for SharedBuffer.
 * True when the containers are identical.
 */
inline bool operator==(const SharedBuffer &lhs, const SharedBuffer &rhs);

/*!
 * Inquality operator for SharedBuffer.
 * True when the containers are not identical.
 */
inline bool operator!=(const SharedBuffer &lhs, const SharedBuffer &rhs);

} //namespace Pothos

inline size_t Pothos::SharedBuffer::getAddress(void) const
{
    return _address;
}

inline size_t Pothos::SharedBuffer::getLength(void) const
{
    return _length;
}

inline size_t Pothos::SharedBuffer::getAlias(void) const
{
    return _alias;
}

inline size_t Pothos::SharedBuffer::getAliasOffset(void) const
{
    return _alias ? _alias - _address : 0;
}

inline void Pothos::SharedBuffer::setAlias(const size_t alias)
{
    _alias = alias;
}

inline size_t Pothos::SharedBuffer::getEnd(void) const
{
    return _address + _length;
}

inline bool Pothos::SharedBuffer::unique(void) const
{
    return _container.unique();
}

inline size_t Pothos::SharedBuffer::useCount(void) const
{
    return _container.use_count();
}

inline Pothos::SharedBuffer::operator bool(void) const
{
    return bool(_container);
}

inline const std::shared_ptr<void> &Pothos::SharedBuffer::getContainer(void) const
{
    return _container;
}

inline bool Pothos::operator==(const SharedBuffer &lhs, const SharedBuffer &rhs)
{
    return lhs.getContainer() == rhs.getContainer();
}

inline bool Pothos::operator!=(const SharedBuffer &lhs, const SharedBuffer &rhs)
{
    return !(lhs == rhs);
}
