// Copyright (c) 2013-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/BufferPool.hpp>

Pothos::BufferPool::BufferPool(void):
    _minBuffSize(0)
{
    return;
}

void Pothos::BufferPool::clear(void)
{
    _minBuffSize = 0;
    _buffs.clear();
}

const Pothos::BufferChunk &Pothos::BufferPool::get(const size_t numBytes)
{
    //user asked for a larger buffer size -- drop all entries
    if (numBytes > _minBuffSize)
    {
        _buffs.clear();
        _minBuffSize = numBytes;
    }

    //find the first buffer where we hold the only copy
    for (size_t i = 0; i < _buffs.size(); i++)
    {
        if (_buffs[i].getBuffer().unique()) return _buffs[i];
    }

    //otherwise make a new buffer
    _buffs.emplace_back(_minBuffSize);
    return _buffs.back();
}
