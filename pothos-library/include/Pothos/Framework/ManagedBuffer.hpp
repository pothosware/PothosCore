//
// Framework/ManagedBuffer.hpp
//
// A ManagedBuffer is checked-out and automatically returned to a BufferManager.
//
// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Framework/SharedBuffer.hpp>
#include <memory> //shared_ptr

namespace Pothos {

//! Forward declare buffer manager
class BufferManager;

/*!
 * A ManagedBuffer is a buffer that interacts with a BufferManager.
 * The ManagedBuffer maintains a reference count, and when no more
 * copies are held by the user, the ManagedBuffer will be returned
 * to the BufferManager; unless the BufferManager has been deleted.
 */
class POTHOS_API ManagedBuffer
{
public:

    /*!
     * Create a null buffer.
     * All fields are initialized to zero.
     */
    ManagedBuffer(void);

    /*!
     * Is this managed buffer valid?
     * \return true if it holds an allocation.
     */
    pothos_explicit operator bool(void) const;

    /*!
     * Reset any reference held by the managed buffer.
     * This managed buffer will become a null buffer.
     */
    void reset(void);

    /*!
     * Reset the managed buffer for its next usage.
     * \param manager shared pointer to the buffer manager
     * \param buff the shared buffer with memory and length
     * \param slabIndex an identifying index used with slabs
     */
    void reset(std::shared_ptr<BufferManager> manager, const SharedBuffer &buff, const size_t slabIndex = 0);

    /*!
     * Get the internal buffer as set by the manager
     * and which cannot be changed by the caller.
     * \return a const reference to the buffer
     */
    const SharedBuffer &getBuffer(void) const;

    /*!
     * Get the slab index associated with this buffer.
     * The slab index related to implementation specifics.
     * Typically, this index is only used when the underlying
     * buffer is sourced from a continuous slab of memory.
     */
    size_t getSlabIndex(void) const;

    //! ManagedBuffer destructor
    ~ManagedBuffer(void);

    //! ManagedBuffer copy constructor
    ManagedBuffer(const ManagedBuffer &obj);

    //! ManagedBuffer move constructor
    ManagedBuffer(ManagedBuffer &&obj);

    //! ManagedBuffer copy assignment
    ManagedBuffer &operator=(const ManagedBuffer &obj);

    //! ManagedBuffer move assignment
    ManagedBuffer &operator=(ManagedBuffer &&obj);

    //! Get the associated buffer manager
    std::shared_ptr<BufferManager> getBufferManager(void) const;

private:
    struct Impl; Impl *_impl;
    POTHOS_API friend bool operator==(const ManagedBuffer &lhs, const ManagedBuffer &rhs);
};

POTHOS_API bool operator==(const ManagedBuffer &lhs, const ManagedBuffer &rhs);

} //namespace Pothos

inline Pothos::ManagedBuffer::operator bool(void) const
{
    return _impl != nullptr;
}
