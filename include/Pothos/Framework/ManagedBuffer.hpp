///
/// \file Framework/ManagedBuffer.hpp
///
/// A ManagedBuffer is checked-out and automatically returned to a BufferManager.
///
/// \copyright
/// Copyright (c) 2013-2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Framework/SharedBuffer.hpp>
#include <memory> //shared_ptr
#include <atomic>
#include <cassert>

namespace Pothos {

//! Forward declares
class BufferManager;
class BufferChunk;

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

    //! sortable operator for ManagedBuffer
    bool operator<(const ManagedBuffer &) const;

    /*!
     * Is the reference to the managed buffer unique?
     * \return true if this is the only copy of the managed buffer
     */
    bool unique(void) const;

    /*!
     * The number of copies of the managed buffer.
     */
    size_t useCount(void) const;

    /*!
     * Set the next contiguous buffer in the chain.
     */
    void setNextBuffer(const ManagedBuffer &next);

    /*!
     * Get the next contiguous buffer in the chain.
     * Or return a null managed buffer if there is none.
     */
    ManagedBuffer getNextBuffer(void) const;

private:
    friend BufferChunk;
    struct Impl; Impl *_impl;
    ManagedBuffer(Impl *impl);
    POTHOS_API friend bool operator==(const ManagedBuffer &lhs, const ManagedBuffer &rhs);
};

/*!
 * Equality operator for ManagedBuffer.
 * True when the containers are identical.
 */
inline bool operator==(const ManagedBuffer &lhs, const ManagedBuffer &rhs);

} //namespace Pothos

struct Pothos::ManagedBuffer::Impl
{
    Impl(void);

    std::atomic<int> counter;
    std::weak_ptr<BufferManager> weakManager;
    SharedBuffer buffer;
    size_t slabIndex;
    Pothos::ManagedBuffer::Impl *nextBuffer;

    void incr(void)
    {
        counter++;
    }

    void decr(void)
    {
        //decrement the counter, and handle the last ref case
        if (counter.fetch_sub(1) == 1) this->cleanup();
    }

    void cleanup(void);
};

inline Pothos::ManagedBuffer::ManagedBuffer(void):
    _impl(nullptr)
{
    return;
}

inline Pothos::ManagedBuffer::operator bool(void) const
{
    return _impl != nullptr;
}

inline void Pothos::ManagedBuffer::reset(void)
{
    if (_impl != nullptr) _impl->decr();
    _impl = nullptr;
}

inline const Pothos::SharedBuffer &Pothos::ManagedBuffer::getBuffer(void) const
{
    assert(*this);
    return _impl->buffer;
}

inline size_t Pothos::ManagedBuffer::getSlabIndex(void) const
{
    assert(*this);
    return _impl->slabIndex;
}

inline Pothos::ManagedBuffer::~ManagedBuffer(void)
{
    if (_impl != nullptr) _impl->decr();
}

inline Pothos::ManagedBuffer::ManagedBuffer(const ManagedBuffer &obj):
    _impl(obj._impl)
{
    if (_impl != nullptr) _impl->incr();
}

inline Pothos::ManagedBuffer::ManagedBuffer(ManagedBuffer &&obj):
    _impl(obj._impl)
{
    obj._impl = nullptr;
}

inline Pothos::ManagedBuffer &Pothos::ManagedBuffer::operator=(const ManagedBuffer &obj)
{
    if (_impl != nullptr) _impl->decr();
    this->_impl = obj._impl;
    if (_impl != nullptr) _impl->incr();
    return *this;
}

inline Pothos::ManagedBuffer &Pothos::ManagedBuffer::operator=(ManagedBuffer &&obj)
{
    if (_impl != nullptr) _impl->decr();
    this->_impl = obj._impl;
    obj._impl = nullptr;
    return *this;
}

inline std::shared_ptr<Pothos::BufferManager> Pothos::ManagedBuffer::getBufferManager(void) const
{
    assert(*this);
    return _impl->weakManager.lock();
}

inline bool Pothos::ManagedBuffer::operator<(const ManagedBuffer &rhs) const
{
    return _impl < rhs._impl;
}

inline bool Pothos::ManagedBuffer::unique(void) const
{
    return this->useCount() == 1;
}

inline size_t Pothos::ManagedBuffer::useCount(void) const
{
    if (*this) return _impl->counter;
    return 0;
}

inline void Pothos::ManagedBuffer::setNextBuffer(const ManagedBuffer &next)
{
    assert(*this);
    _impl->nextBuffer = next._impl;
}

inline Pothos::ManagedBuffer Pothos::ManagedBuffer::getNextBuffer(void) const
{
    assert(*this);
    return Pothos::ManagedBuffer(_impl->nextBuffer);
}

inline Pothos::ManagedBuffer::ManagedBuffer(Impl *impl):
    _impl(impl)
{
    if (_impl != nullptr) _impl->incr();
}

inline bool Pothos::operator==(const ManagedBuffer &lhs, const ManagedBuffer &rhs)
{
    return lhs._impl == rhs._impl;
}
