///
/// \file Util/SpinLock.hpp
///
/// A simple C++11 spin lock implementation.
///
/// \copyright
/// Copyright (c) 2014-2015 Josh Blum
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
 * This lock can be used with std::unique_lock<Pothos::Util::SpinLock>
 */
class POTHOS_API SpinLock
{
public:

    //! Create a new unlocked spin lock
    SpinLock(void)
    {
        this->unlock();
    }

    //! Lock the spin lock, block if already locked
    void lock(void)
    {
        while (_lock.test_and_set(std::memory_order_acquire)){}
    }

    //! Unlock the spin lock (should be already locked)
    void unlock(void)
    {
        _lock.clear(std::memory_order_release);
    }

private:
    std::atomic_flag _lock;
};

} //namespace Util
} //namespace Pothos
