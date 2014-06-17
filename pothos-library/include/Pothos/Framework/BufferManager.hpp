//
// Framework/BufferManager.hpp
//
// TODO
//
// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Framework/ManagedBuffer.hpp>
#include <memory> //shared_ptr
#include <string>
#include <functional>

namespace Pothos {

/*!
 * BufferManagerArgs - constructor arguments for BufferManager.
 */
struct POTHOS_API BufferManagerArgs
{
    BufferManagerArgs(void);

    /*!
     * The number of managed buffers available from the manager.
     * Buffers are checked into and out of the manager frequently.
     * A small number of buffers are needed to allow for parallelism,
     * so buffers can be checked out while other buffers are in use.
     * Default: 4 buffers
     */
    size_t numBuffers;

    /*!
     * The number of bytes available per each managed buffer.
     * Default: 8 kibibytes
     */
    size_t bufferSize;

    /*!
     * The NUMA node affinity for the generic slab allocator.
     * This argument is not used for the special-case managers.
     * Default: -1 or unspecified affinity
     */
    long nodeAffinity;
};

/*!
 * A BufferManager has a queue-like interface to manage buffers.
 * Since buffers can represent special DMA memory,
 * the buffer manager is responsible for DMA hooks.
 */
class POTHOS_API BufferManager
{
public:

    typedef std::shared_ptr<BufferManager> Sptr;

    /*!
     * The BufferManager factory -- makes a new BufferManager given the factory name.
     * Plugins for custom BufferManagers should be located in
     * the plugin registry: /framework/buffer_manager/[name]
     * \throws BufferManagerFactoryError if the factory function fails.
     * \param name the name of a BufferManager factory in the plugin tree
     * \param args the buffer manager init arguments
     * \return a new shared pointer to a buffer manager
     */
    static Sptr make(const std::string &name, const BufferManagerArgs &args);

    /*!
     * Init is called once at factory time to initialize the buffers.
     */
    virtual void init(const BufferManagerArgs &args) = 0;

    /*!
     * Is the manager empty?
     * \return true if no buffers are available
     */
    virtual bool empty(void) const = 0;

    /*!
     * Get a reference to the front buffer.
     * front().address will be the start of a valid buffer,
     * front().length will be the number of bytes available.
     * The caller sets length to indicate bytes used.
     * \return the buffer at the queue head
     */
    virtual const ManagedBuffer &front(void) const = 0;

    /*!
     * Pop bytes from the front buffer.
     * This operation removes available bytes from the manager.
     * Calling pop removes a managed buffer from an internal queue;
     * the bytes removed are only restored to the manager via push.
     *
     * If this manager is responsible for DMA writes,
     * the pop operation should cause a write operation.
     * If this manager is responsible for DMA reads,
     * this pop operation should cause a read operation.
     *
     * \param numBytes the number of bytes to remove
     */
    virtual void pop(const size_t numBytes) = 0;

    /*!
     * Push returns a buffer to the manager.
     * The buffer may be available via a call to front()
     * depending upon the implementation of the manager.
     * \throw BufferPushError if buffer does not belong to this manager.
     * \param buff the buffer to return
     */
    virtual void push(const ManagedBuffer &buff) = 0;

    /*!
     * Push external returns a buffer to the manager through a callback.
     * This call is used to return a buffer from a different thread context.
     * The callback implements the thread-safe delivery mechanism.
     */
    void pushExternal(const ManagedBuffer &buff);

    /*!
     * Set the callback for use with the pushExternal API call.
     */
    void setCallback(const std::function<void(const ManagedBuffer &)> &callback);

private:
    std::function<void(const ManagedBuffer &)> _callback;
};

} //namespace Pothos

inline void Pothos::BufferManager::pushExternal(const ManagedBuffer &buff)
{
    if (_callback) _callback(buff);
    else this->push(buff);
}
