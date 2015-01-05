// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
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
        _changeFlagged(false),
        _externalAcquired(0),
        _contextAcquired(0)
    {
        _internalAcquired.clear(std::memory_order_release);
    }

    ~ActorInterface(void){}

    /*!
     * External callers from outside of the thread context
     * may use this to acquire exclusive access to the actor.
     */
    void externalCallAcquire(void);

    //! Release external caller's exclusive access to the actor
    void externalCallRelease(void);

    /*!
     * Acquire exclusive access to the actor context.
     * \return true when acquired, false for timeout
     */
    bool workerThreadAcquire(void);

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

private:

    //! called in release/flag to wake up waiters
    void notifyWaiters(void);

    std::atomic<bool> _changeFlagged;
    std::atomic<size_t> _externalAcquired;
    std::atomic_flag _internalAcquired;
    std::atomic<size_t> _contextAcquired;
    std::mutex _mutex;
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
    std::unique_lock<std::mutex> lock(_mutex);
    _externalAcquired++;
    while (_contextAcquired != 0)
    {
        _cond.wait(lock);
        //_cond.wait_for(lock, std::chrono::milliseconds(1));
    }
    _contextAcquired++;
}

inline void ActorInterface::externalCallRelease(void)
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _contextAcquired--;
        _externalAcquired--;
        _changeFlagged = true;
    }
    this->notifyWaiters();
}

inline bool ActorInterface::workerThreadAcquire(void)
{
    //when used in pool mode, this call returns ASAP when another thread is working
    if (not _internalAcquired.test_and_set(std::memory_order_acquire)) return false;

    std::unique_lock<std::mutex> lock(_mutex);
    while (not _changeFlagged or _contextAcquired != 0 or _externalAcquired > 0)
    {
        //_cond.wait(lock);
        if (_cond.wait_for(lock, std::chrono::milliseconds(1)) == std::cv_status::timeout) return false;
    }
    _changeFlagged = false;
    _contextAcquired++;
    return true;
}

inline void ActorInterface::workerThreadRelease(void)
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _contextAcquired--;
    }
    this->notifyWaiters();
    _internalAcquired.clear(std::memory_order_release);
}

inline void ActorInterface::flagExternalChange(void)
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _changeFlagged = true;
    }
    this->notifyWaiters();
}

inline void ActorInterface::flagInternalChange(void)
{
    _changeFlagged = true;
}

inline void ActorInterface::notifyWaiters(void)
{
    _cond.notify_all();
}
