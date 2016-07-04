///
/// \file Framework/BufferPool.hpp
///
/// A simple buffer pool with re-usable buffers.
///
/// \copyright
/// Copyright (c) 2016-2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Framework/BufferChunk.hpp>
#include <vector>

namespace Pothos {

/*!
 * The simple buffer pool holds a collection of re-usable buffers.
 * When the client requests a particular buffer size from the pool,
 * the pool first looks for an existing and unused buffer
 * that matches the requested size, or allocates a new buffer.
 */
class POTHOS_API BufferPool
{
public:

    //! Construct an empty buffer pool
    BufferPool(void);

    /*!
     * Clear all existing entries in the pool.
     */
    void clear(void);

    /*!
     * Get a buffer from the pool or make one if none available.
     * \param numBytes the size of the requested buffer in bytes
     * \return an available buffer chunk of at least numBytes size
     */
    const Pothos::BufferChunk &get(const size_t numBytes);

private:
    size_t _minBuffSize;
    std::vector<Pothos::BufferChunk> _buffs;
};

} //namespace Pothos
