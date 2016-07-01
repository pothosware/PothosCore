// Copyright (c) 2013-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/ManagedBuffer.hpp>
#include <Pothos/Framework/BufferManager.hpp>
#include <atomic>
#include <cassert>

struct Pothos::ManagedBuffer::Impl
{
    Impl(void):
        counter(1),
        slabIndex(0),
        nextBuffer(nullptr)
    {
        return;
    }
    std::atomic<int> counter;
    std::weak_ptr<BufferManager> weakManager;
    SharedBuffer buffer;
    size_t slabIndex;
    Pothos::ManagedBuffer::Impl *nextBuffer;
};

Pothos::ManagedBuffer::ManagedBuffer(void):
    _impl(nullptr)
{
    return;
}

Pothos::ManagedBuffer::ManagedBuffer(Impl *impl):
    _impl(impl)
{
    _incrRef();
}

void Pothos::ManagedBuffer::reset(void)
{
    _decrRef();
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
    _decrRef();
}

Pothos::ManagedBuffer::ManagedBuffer(const ManagedBuffer &obj):
    _impl(obj._impl)
{
    _incrRef();
}

Pothos::ManagedBuffer::ManagedBuffer(ManagedBuffer &&obj):
    _impl(obj._impl)
{
    obj._impl = nullptr;
}

Pothos::ManagedBuffer &Pothos::ManagedBuffer::operator=(const ManagedBuffer &obj)
{
    _decrRef();
    this->_impl = obj._impl;
    _incrRef();
    return *this;
}

Pothos::ManagedBuffer &Pothos::ManagedBuffer::operator=(ManagedBuffer &&obj)
{
    _decrRef();
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
    assert(*this);
    _impl->nextBuffer = next._impl;
}

Pothos::ManagedBuffer Pothos::ManagedBuffer::getNextBuffer(void) const
{
    assert(*this);
    return Pothos::ManagedBuffer(_impl->nextBuffer);
}

void Pothos::ManagedBuffer::_incrRef(void)
{
    if (_impl != nullptr) _impl->counter++;
}

void Pothos::ManagedBuffer::_decrRef(void)
{
    //decrement the counter, and handle the last ref case
    if (_impl != nullptr and _impl->counter.fetch_sub(1) == 1)
    {
        //there is a manager to push to, otherwise delete
        std::shared_ptr<BufferManager> manager = _impl->weakManager.lock();
        if (manager) manager->pushExternal(*this);
        else delete _impl;
    }
}
