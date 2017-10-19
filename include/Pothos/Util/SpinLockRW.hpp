///
/// \file Util/SpinLockRW.hpp
///
/// A read/write spinlock implementation
///
/// \copyright
/// Copyright (c) 2017-2017 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <thread>
#include <atomic>

namespace Pothos {
namespace Util {

/*!
 * Implementation of SharedLock
 * Switch to std::shared_lock for C++14
 */
template <typename T>
class SharedLock
{
public:
    SharedLock(T &mutex):
        _mutex(mutex)
    {
        _mutex.lock_shared();
    }

    ~SharedLock(void)
    {
        _mutex.unlock_shared();
    }

private:
    T &_mutex;
};

/*!
 * A spin lock that supports multiple readers + single writer.
 * This lock is optimized for infrequent writes and frequent reading.
 * Its primarily used in the core library for certain plugin hooks
 * which require write locks during the plugin's loader hooks but
 * then requires almost exclusively reads during runtime operation.
 *
 * - For writers, use with std::lock_guard<Pothos::Util::SpinLock>
 * - For readers, use with Pothos::Util::SpinLockRW::SharedLock
 */
class POTHOS_API SpinLockRW
{
public:

    //! Convenient typedef for shared lock type
    typedef Pothos::Util::SharedLock<SpinLockRW> SharedLock;

    //! Create a new unlocked spin lock
    SpinLockRW(void);

    //! Try to lock shared, return true for lock
    bool try_lock_shared(void) noexcept;

    //! Lock for multiple reader access
    void lock_shared(void) noexcept;

    //! Unlock from multiple reader access
    void unlock_shared(void) noexcept;

    //! Try to lock, return true for lock
    bool try_lock(void) noexcept;

    //! Lock for single writer access
    void lock(void) noexcept;

    //! Unlock single writer access
    void unlock(void) noexcept;

private:
    enum : unsigned {WRITER_LOCK = unsigned(~0), UNLOCKED = 0};
    std::atomic<unsigned> _lock;
};

} //namespace Util
} //namespace Pothos


inline Pothos::Util::SpinLockRW::SpinLockRW(void)
{
    this->unlock();
}

inline bool Pothos::Util::SpinLockRW::try_lock_shared(void) noexcept
{
    //true when the expected condition is not write lock
    //and swap in the value of expected +1 (additional reader)
    unsigned expected = _lock.load(std::memory_order_acquire);
    return expected != WRITER_LOCK and _lock.compare_exchange_weak(expected, expected+1, std::memory_order_acq_rel);
}

inline void Pothos::Util::SpinLockRW::lock_shared(void) noexcept
{
    size_t count(0);
    while (not this->try_lock_shared())
    {
        if (++count > 1024) std::this_thread::yield();
    }
}

inline void Pothos::Util::SpinLockRW::unlock_shared(void) noexcept
{
    //decrement the reader count by 1
    _lock.fetch_sub(1, std::memory_order_release);
}

inline bool Pothos::Util::SpinLockRW::try_lock(void) noexcept
{
    //true when the expected condition is unlocked (no readers)
    //and swap in the value of write lock (one writer)
    unsigned expected = UNLOCKED;
    return _lock.compare_exchange_weak(expected, WRITER_LOCK, std::memory_order_acq_rel);
}

inline void Pothos::Util::SpinLockRW::lock(void) noexcept
{
    size_t count(0);
    while (not this->try_lock())
    {
        if (++count > 1024) std::this_thread::yield();
    }
}

inline void Pothos::Util::SpinLockRW::unlock(void) noexcept
{
    //restore to unlocked (no writer, no readers)
    _lock.store(UNLOCKED, std::memory_order_release);
}
