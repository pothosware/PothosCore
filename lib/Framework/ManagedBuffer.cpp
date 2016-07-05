// Copyright (c) 2013-2016 Josh Blum
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
    std::shared_ptr<BufferManager> manager = weakManager.lock();
    if (manager) manager->pushExternal(ManagedBuffer(this));
    else delete this;
}

void Pothos::ManagedBuffer::reset(BufferManager::Sptr manager, const SharedBuffer &buff, const size_t slabIndex)
{
    if (_impl == nullptr) _impl = new Impl();
    _impl->buffer = buff;
    _impl->slabIndex = slabIndex;
    _impl->weakManager = manager;
}

