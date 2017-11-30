// Copyright (c) 2013-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/ManagedBuffer.hpp>
#include <Pothos/Framework/BufferManager.hpp>

Pothos::ManagedBuffer::Impl::Impl(void):
    counter(1),
    slabIndex(0),
    nextBuffer(nullptr)
{
    return;
}

void Pothos::ManagedBuffer::Impl::cleanup(void)
{
    //there is a manager to push to, otherwise delete
    BufferManager::Sptr manager = weakManager.lock();
    if (manager)
    {
        ManagedBuffer mb;
        mb._impl = this;
        manager->pushExternal(mb);
        mb._impl = nullptr;
    }
    else delete this;
}

Pothos::ManagedBuffer::ManagedBuffer(const SharedBuffer &buff):
    _impl(new Impl())
{
    _impl->buffer = buff;
}

void Pothos::ManagedBuffer::reset(BufferManager::Sptr manager, const SharedBuffer &buff, const size_t slabIndex)
{
    if (_impl == nullptr) _impl = new Impl();
    _impl->buffer = buff;
    _impl->slabIndex = slabIndex;
    _impl->weakManager = manager;
}

