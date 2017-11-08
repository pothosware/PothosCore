// Copyright (c) 2015-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Util/SpinLock.hpp>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>

/*!
 * Allow waiting when idle iterations surpasses this count.
 * At present this is a small, non-zero number which helps
 * to avoid waiting and locking for threads that are active.
 * In the future, investigate hybrid between wait and non-wait
 * by making this value configurable based on user setting
 * or modifying it based on activity and work indicators.
 */
#define WAIT_ITERS 8

/*!
 * The implementation of the exclusive access to the actor.
 */
class ActorInterface
{
public:

    ActorInterface(void):
        _waitModeEnabled(true),
        _externalAcquired(0),
        _idleIters(0)
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
    bool _workerThreadAcquireWait(const bool waitEnabled);
    bool _inExternalCall(void);
    bool _waitModeEnabled;
    std::atomic_flag _changeFlagged;
    std::atomic<unsigned> _externalAcquired;
    std::mutex _acquireMutex;
    std::atomic<unsigned> _idleIters;
    Pothos::Util::SpinLock _extCallLock;
    std::condition_variable _acquireCond;
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

inline bool ActorInterface::_inExternalCall(void)
{
    return _externalAcquired.load(std::memory_order_acquire) != 0;
}

inline void ActorInterface::externalCallAcquire(void)
{
    //wait in a loop to acquire the call lock
    //rather than assume notify will be reliable,
    //we wait with a timeout and recheck the condition
    std::unique_lock<std::mutex> lock(_acquireMutex);
    _externalAcquired++;
    while (not _extCallLock.try_lock())
    {
        _acquireCond.wait_for(lock, std::chrono::milliseconds(1));
    }
}

inline void ActorInterface::externalCallRelease(void)
{
    //release the lock and notify any potential waiters
    //flag a change so the worker can re-evaluate state after the call
    _externalAcquired--;
    _extCallLock.unlock();
    this->flagInternalChange();
    _acquireCond.notify_all();
}

inline bool ActorInterface::workerThreadAcquire(const bool waitEnabled)
{
    //Attempt to acquire the change notification:
    //The wait enabled is only set after an iteration threshold has been passed,
    //or enable waiting when an external call may be active to mitigate overhead.
    //And we always clear the iteration count when the change has been acquired.
    if (_workerThreadAcquireWait(waitEnabled and (this->_inExternalCall() or
        _idleIters.fetch_add(1, std::memory_order_acq_rel) > WAIT_ITERS)))
    {
        _idleIters.store(0, std::memory_order_release); //clear
        if (_extCallLock.try_lock()) return true; //lock out external calls
        this->flagInternalChange(); //or busy in a call, re-flag the change
    }
    return false;
}

inline bool ActorInterface::_workerThreadAcquireWait(const bool waitEnabled)
{
    //Lock and wait on external calls to complete or activity to be flagged.
    if (waitEnabled)
    {
        std::unique_lock<std::mutex> lock(_acquireMutex);
        return _acquireCond.wait_for(lock, std::chrono::milliseconds(1), [this]
        {
            if (this->_inExternalCall()) return false;
            return not _changeFlagged.test_and_set(std::memory_order_acquire);
        });
    }

    //atomically acquire the change notification without locking
    return (not _changeFlagged.test_and_set(std::memory_order_acquire));
}

inline void ActorInterface::workerThreadRelease(void)
{
    //release call lock and notify any enqueued callers
    _extCallLock.unlock();
    if (this->_inExternalCall()) _acquireCond.notify_all();
}

inline void ActorInterface::flagExternalChange(void)
{
    //asynchronous indication
    _changeFlagged.clear(std::memory_order_release);

    //wake a blocked thread to process the change
    //lock the mutex and mark the changed flag again
    //so that the thread will see the change on wake
    if (_idleIters.load(std::memory_order_acquire) >= WAIT_ITERS)
    {
        std::lock_guard<std::mutex> lock(_acquireMutex);
        _changeFlagged.clear(std::memory_order_release);
        _acquireCond.notify_one();
    }
}

inline void ActorInterface::wakeNoChange(void)
{
    //called by the thread environment at cleanup time
    //to cause workerThreadAcquire() to wakeup and exit
    _acquireCond.notify_all();
}

inline void ActorInterface::flagInternalChange(void)
{
    _changeFlagged.clear(std::memory_order_release);
}
