// Copyright (c) 2013-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/ManagedBuffer.hpp>
#include <atomic>

struct Pothos::ManagedBuffer::Impl
{
    Impl(void):
        counter(1),
        slabIndex(0)
    {
        return;
    }
    std::atomic<int> counter;
    std::weak_ptr<BufferManager> weakManager;
    SharedBuffer buffer;
    size_t slabIndex;
    Pothos::ManagedBuffer nextBuffer;
};
