// Copyright (c) 2013 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Object/ObjectImpl.hpp>
#include <Theron/DefaultAllocator.h>
#include <Poco/SingletonHolder.h>

static Theron::DefaultAllocator &getAllocator(void)
{
    static Poco::SingletonHolder<Theron::DefaultAllocator> sh;
    return *sh.get();
}

void *Pothos::Detail::ObjectContainer::operator new(const size_t size)
{
    return getAllocator().Allocate(size);
}

void Pothos::Detail::ObjectContainer::operator delete(void *memory, const size_t)
{
    return getAllocator().Free(memory);
}
