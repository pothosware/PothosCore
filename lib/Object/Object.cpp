// Copyright (c) 2013-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Object/ObjectImpl.hpp>
#include <Pothos/Object/Exception.hpp>
#include <Pothos/Util/TypeInfo.hpp>
#include <Poco/Format.h>
#include <cassert>

/***********************************************************************
 * NullObject impl
 **********************************************************************/
Pothos::NullObject::NullObject(void)
{
    return;
}

Pothos::NullObject::~NullObject(void)
{
    return;
}

/***********************************************************************
 * Object container
 **********************************************************************/
Pothos::Detail::ObjectContainer::ObjectContainer(const std::type_info &type):
    internal(nullptr),
    type(type),
    counter(1)
{
   return;
}

Pothos::Detail::ObjectContainer::~ObjectContainer(void)
{
    return;
}

static void incr(Pothos::Detail::ObjectContainer *o)
{
    if (o == nullptr) return;
    o->counter.fetch_add(1, std::memory_order_relaxed);
}

static void decr(Pothos::Detail::ObjectContainer *o)
{
    if (o == nullptr) return;
    if (o->counter.fetch_sub(1, std::memory_order_release) == 1)
    {
        std::atomic_thread_fence(std::memory_order_acquire);
        delete o;
    }
}

void Pothos::Detail::throwExtract(const Pothos::Object &obj, const std::type_info &type)
{
    assert(obj.type() != type);
    throw ObjectConvertError("Pothos::Object::extract()",
        Poco::format("Cannot convert Object of type %s to %s",
        obj.getTypeString(), Util::typeInfoToString(type)));
}

Pothos::Detail::ObjectContainer *Pothos::Detail::makeObjectContainer(const char *s)
{
    return makeObjectContainer(std::string(s));
}

/***********************************************************************
 * Object impl
 **********************************************************************/
Pothos::Object::Object(void):
    _impl(nullptr)
{
    assert(not *this);
}

Pothos::Object::Object(const Object &obj):
    _impl(obj._impl)
{
    incr(_impl);
}

Pothos::Object::~Object(void)
{
    decr(_impl);
}

Pothos::Object::operator bool(void) const
{
    return _impl != nullptr;
}

Pothos::Object &Pothos::Object::operator=(const Object &rhs)
{
    decr(_impl);
    _impl = rhs._impl;
    incr(_impl);
    return *this;
}

Pothos::Object &Pothos::Object::operator=(Object &&rhs)
{
    decr(_impl);
    _impl = rhs._impl;
    rhs._impl = nullptr;
    return *this;
}

bool Pothos::Object::unique(void) const
{
    return _impl->counter.load(std::memory_order_relaxed) == 1;
}

std::string Pothos::Object::getTypeString(void) const
{
    return Util::typeInfoToString(this->type());
}

bool Pothos::Object::equals(const Object &obj) const
{
    try
    {
        return this->compareTo(obj) == 0;
    }
    catch (const Pothos::ObjectCompareError &)
    {
        return this->hashCode() == obj.hashCode();
    }
}

bool Pothos::Object::operator<(const Pothos::Object &obj) const
{
    try
    {
        return this->compareTo(obj) < 0;
    }
    catch (const Pothos::ObjectCompareError &)
    {
        return this->hashCode() < obj.hashCode();
    }
}

bool Pothos::Object::operator>(const Pothos::Object &obj) const
{
    try
    {
        return this->compareTo(obj) > 0;
    }
    catch (const Pothos::ObjectCompareError &)
    {
        return this->hashCode() > obj.hashCode();
    }
}

#include <Pothos/Managed.hpp>

static auto managedObject = Pothos::ManagedClass()
    .registerConstructor<Pothos::Object>()
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Object, unique))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Object, toString))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Object, getTypeString))
    .commit("Pothos/Object");
