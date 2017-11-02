///
/// \file Util/SpinLock.hpp
///
/// A simple C++11 spin lock implementation.
///
/// \copyright
/// Copyright (c) 2014-2017 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <atomic>

namespace Pothos {
namespace Util {

/*!
 * A generic spin lock implementation using std::atomic.
 *
 * There are no embedded back-off or sleep conditions in this lock,
 * therefore only use this lock to protect very brief code sections.
 *
 * This lock can be used with std::lock_guard<Pothos::Util::SpinLock>
 */
class POTHOS_API SpinLock
{
public:

    //! Create a new unlocked spin lock
    SpinLock(void);

    //! Try to lock, return true for lock
    bool try_lock(void) noexcept;

    //! Lock the spin lock, block if already locked
    void lock(void) noexcept;

    //! Unlock the spin lock (should be already locked)
    void unlock(void) noexcept;

private:
    std::atomic_flag _lock;
};

} //namespace Util
} //namespace Pothos

#if defined(_MSC_VER) and defined(_M_X64)
#include <intrin.h>
#endif

inline Pothos::Util::SpinLock::SpinLock(void)
{
    this->unlock();
}

inline bool Pothos::Util::SpinLock::try_lock(void) noexcept
{
    return not _lock.test_and_set(std::memory_order_acquire);
}

inline void Pothos::Util::SpinLock::lock(void) noexcept
{
    while (not this->try_lock())
    {
        #if defined(_MSC_VER) and defined(_M_X64)
            _mm_pause();
        #elif defined(__GNUC__) || defined(__clang__)
            #if defined(__i386__) || defined(__x86_64__)
            __builtin_ia32_pause();
            #elif defined(__GNUC__) and defined(__arm__)
            __yield();
            #endif
        #endif
    }
}

inline void Pothos::Util::SpinLock::unlock(void) noexcept
{
    _lock.clear(std::memory_order_release);
}
