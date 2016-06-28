// Copyright (c) 2013-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "ManagedBufferImpl.hpp"
#include <Pothos/Framework/ManagedBuffer.hpp>
#include <Pothos/Framework/BufferManager.hpp>
#include <cassert>

Pothos::ManagedBuffer::ManagedBuffer(void):
    _impl(nullptr)
{
    return;
}

void Pothos::ManagedBuffer::reset(void)
{
    //decrement the counter, and handle the last ref case
    if (_impl != nullptr and _impl->counter.fetch_sub(1) == 1)
    {
        //there is a manager to push to, otherwise delete
        std::shared_ptr<BufferManager> manager = _impl->weakManager.lock();
        if (manager) manager->pushExternal(*this);
        else delete _impl;
    }

    //zero out the class members
    _impl = nullptr;
}

void Pothos::ManagedBuffer::reset(BufferManager::Sptr manager, const SharedBuffer &buff, const size_t slabIndex)
{
    if (_impl == nullptr) _impl = new Impl();
    _impl->buffer = buff;
    _impl->slabIndex = slabIndex;
    _impl->weakManager = manager;
}

Pothos::ManagedBuffer::~ManagedBuffer(void)
{
    this->reset();
}

Pothos::ManagedBuffer::ManagedBuffer(const ManagedBuffer &obj):
    _impl(nullptr)
{
    *this = obj;
}

Pothos::ManagedBuffer::ManagedBuffer(ManagedBuffer &&obj):
    _impl(nullptr)
{
    *this = obj;
}

Pothos::ManagedBuffer &Pothos::ManagedBuffer::operator=(const ManagedBuffer &obj)
{
    this->reset();
    this->_impl = obj._impl;
    if (_impl != nullptr) _impl->counter.fetch_add(1);
    return *this;
}

Pothos::ManagedBuffer &Pothos::ManagedBuffer::operator=(ManagedBuffer &&obj)
{
    this->reset();
    this->_impl = obj._impl;
    obj._impl = nullptr;
    return *this;
}

const Pothos::SharedBuffer &Pothos::ManagedBuffer::getBuffer(void) const
{
    assert(*this);
    return _impl->buffer;
}

size_t Pothos::ManagedBuffer::getSlabIndex(void) const
{
    assert(*this);
    return _impl->slabIndex;
}

Pothos::BufferManager::Sptr Pothos::ManagedBuffer::getBufferManager(void) const
{
    assert(*this);
    return _impl->weakManager.lock();
}

size_t Pothos::ManagedBuffer::useCount(void) const
{
    if (*this) return _impl->counter;
    return 0;
}

void Pothos::ManagedBuffer::setNextBuffer(const ManagedBuffer &next)
{
    _impl->nextBuffer = next;
}

Pothos::ManagedBuffer &Pothos::ManagedBuffer::getNextBuffer(void) const
{
    return _impl->nextBuffer;
}
