// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Util/SpinLockRW.hpp>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>

/*!
 * The implementation of the exclusive access to the actor.
 */
class ActorInterface
{
public:

    ActorInterface(void):
        _waitModeEnabled(true),
        _numIdleIters(0)
    {
        _changeFlagged.test_and_set();
    }

    virtual ~ActorInterface(void)
    {
        return;
    }

    /*!
     * External callers from outside of the thread context
     * may use this to acquire exclusive access to the actor.
     */
    void externalCallAcquire(void);

    //! Release external caller's exclusive access to the actor
    void externalCallRelease(void);

    /*!
     * Acquire exclusive access to the actor context.
     * \param waitEnabled true to enable CV waiting
     * \return true when acquired, false otherwise
     */
    bool workerThreadAcquire(const bool waitEnabled);

    //! Release exclusive access to the actor context.
    void workerThreadRelease(void);

    /*!
     * An external caller from outside the worker thread context
     * may use this to indicate that a state change has occurred.
     * This call marks the change and wakes up a sleeping thread.
     */
    void flagExternalChange(void);

    /*!
     * An internal call from within the worker thread context
     * may use this call to indicate an internal state change.
     * This call only marks the change because unlike flag external,
     * the worker thread is assumed to be active or making this call.
     */
    void flagInternalChange(void);

    /*!
     * Wake up a potentially sleeping thread without flagging a change.
     */
    void wakeNoChange(void);

    //! Enable or disable use of condition variables
    void enableWaitMode(const bool enb)
    {
        _waitModeEnabled = enb;
    }

private:
    bool _waitModeEnabled;
    std::atomic_flag _changeFlagged;
    std::mutex _contextMutex;
    std::mutex _acquireMutex;
    std::atomic<unsigned> _numIdleIters;
    Pothos::Util::SpinLockRW _spinRW;
    std::condition_variable _cond;
};

/*!
 * A lock-like class for ActorInterface to acquire exclusive access.
 * Use this to lock the actor interface when making external calls.
 */
class ActorInterfaceLock
{
public:
    ActorInterfaceLock(ActorInterface *actor):
        _actor(actor)
    {
        _actor->externalCallAcquire();
    }
    ~ActorInterfaceLock(void)
    {
        _actor->externalCallRelease();
    }
private:
    ActorInterface *_actor;
};

inline void ActorInterface::externalCallAcquire(void)
{
    _contextMutex.lock();
    _spinRW.lock_shared();
}

inline void ActorInterface::externalCallRelease(void)
{
    _contextMutex.unlock();
    _spinRW.unlock_shared();
    this->flagExternalChange();
}

inline bool ActorInterface::workerThreadAcquire(const bool waitEnabled)
{
    //lock fails when there are external calls
    //lock the context mutex to wait nicely for the call to complete
    if (not _spinRW.try_lock())
    {
        std::lock_guard<std::mutex> lock(_contextMutex);
        _spinRW.lock();
    }

    //fast-check for already flagged case
    if (not _changeFlagged.test_and_set(std::memory_order_acquire))
    {
        _numIdleIters.store(0, std::memory_order_release);
        return true;
    }

    //wait mode enabled -- lock and wait on condition variable
    if (waitEnabled and _numIdleIters.fetch_add(1, std::memory_order_acq_rel) > 1)
    {
        std::unique_lock<std::mutex> lock(_acquireMutex);
        if (not _changeFlagged.test_and_set(std::memory_order_acquire))
        {
            _numIdleIters.store(0, std::memory_order_release);
            return true;
        }

        _spinRW.unlock();
        _cond.wait(lock);
        _numIdleIters.store(0, std::memory_order_release);

        if (not _changeFlagged.test_and_set(std::memory_order_acquire))
        {
            _spinRW.lock();
            return true;
        }
    }

    _spinRW.unlock();
    return false;
}

inline void ActorInterface::workerThreadRelease(void)
{
    _spinRW.unlock();
}

inline void ActorInterface::flagExternalChange(void)
{
    //asynchronous indication
    _changeFlagged.clear(std::memory_order_release);

    //wake a blocked thread to process the change
    //lock the mutex and mark the changed flag again
    //so that the thread will see the change on wake
    if (_numIdleIters.load(std::memory_order_acquire) >= 1)
    {
        std::lock_guard<std::mutex> lock(_acquireMutex);
        _changeFlagged.clear(std::memory_order_release);
        _cond.notify_one();
    }
}

inline void ActorInterface::wakeNoChange(void)
{
    if (_numIdleIters.load(std::memory_order_acquire) >= 1)
    {
        _cond.notify_one();
    }
}

inline void ActorInterface::flagInternalChange(void)
{
    _changeFlagged.clear(std::memory_order_release);
}
