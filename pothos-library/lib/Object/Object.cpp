// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Object/ObjectImpl.hpp>
#include <Pothos/Object/Exception.hpp>
#include <Pothos/Util/TypeInfo.hpp>
#include <Poco/AtomicCounter.h>
#include <Poco/Format.h>
#include <cassert>

/***********************************************************************
 * Checks for the template metafoo
 **********************************************************************/
static_assert(std::is_same<
    Pothos::Detail::remove_reference_wrapper<Pothos::Object>::type,
    Pothos::Detail::remove_reference_wrapper<std::reference_wrapper<Pothos::Object>>::type
    >::value, "remove_reference_wrapper broken");

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
Pothos::Detail::ObjectContainer::ObjectContainer(void)
{
    static_assert(
        sizeof(Pothos::Detail::ObjectContainer::counterMem) >= sizeof(Poco::AtomicCounter),
        "Pothos::Detail::ObjectContainer() counter memory too small!");
    this->counter = new (this->counterMem) Poco::AtomicCounter(1);
}

Pothos::Detail::ObjectContainer::~ObjectContainer(void)
{
    return;
}

static void incr(Pothos::Detail::ObjectContainer *o)
{
    if (o == nullptr) return;
    Poco::AtomicCounter &counter = *reinterpret_cast<Poco::AtomicCounter *>(o->counter);
    ++counter;
}

static bool decr(Pothos::Detail::ObjectContainer *o)
{
    if (o == nullptr) return false;
    Poco::AtomicCounter &counter = *reinterpret_cast<Poco::AtomicCounter *>(o->counter);
    return (--counter) == 0;
}

void Pothos::Detail::ObjectContainer::throwExtract(const Pothos::Object &obj, const std::type_info &type)
{
    assert(obj.type() != type);
    throw ObjectConvertError("Pothos::Object::extract()",
        Poco::format("Cannot convert Object of type %s to %s",
        obj.getTypeString(), Util::typeInfoToString(type)));
}

/***********************************************************************
 * Object impl
 **********************************************************************/
Pothos::Object::Object(void):
    _impl(nullptr)
{
    assert(not *this);
}

Pothos::Object Pothos::Object::make(const char *s)
{
    return Object(s);
}

Pothos::Object::Object(const Object &obj):
    _impl(nullptr)
{
    *this = obj;
}

Pothos::Object::Object(Object &obj):
    _impl(nullptr)
{
    *this = obj;
}

Pothos::Object::Object(Object &&obj):
    _impl(nullptr)
{
    *this = obj;
}

Pothos::Object::Object(const Object &&obj):
    _impl(nullptr)
{
    *this = obj;
}

Pothos::Object::Object(const char *s):
    _impl(nullptr)
{
    *this = Object(std::string(s));
}

Pothos::Object::~Object(void)
{
    if (decr(_impl)) delete _impl;
}

Pothos::Object::operator bool(void) const
{
    return _impl != nullptr;
}

Pothos::Object &Pothos::Object::operator=(const Object &rhs)
{
    if (decr(_impl)) delete _impl;
    _impl = rhs._impl;
    incr(_impl);
    return *this;
}

Pothos::Object &Pothos::Object::operator=(Object &&rhs)
{
    if (decr(_impl)) delete _impl;
    _impl = rhs._impl;
    rhs._impl = nullptr;
    return *this;
}

bool Pothos::Object::unique(void) const
{
    Poco::AtomicCounter &counter = *reinterpret_cast<Poco::AtomicCounter *>(_impl->counter);
    return counter.value() == 1;
}

const std::type_info &Pothos::Object::type(void) const
{
    if (_impl == nullptr) return typeid(NullObject);
    return _impl->type();
}

std::string Pothos::Object::toString(void) const
{
    try
    {
        return this->convert<std::string>();
    }
    catch (...)
    {
        return this->getTypeString();
    }
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

/***********************************************************************
 * Object functions
 **********************************************************************/
bool Pothos::operator==(const Object &lhs, const Object &rhs)
{
    return lhs._impl == rhs._impl;
}

#include <Pothos/Managed.hpp>

static auto managedObject = Pothos::ManagedClass()
    .registerConstructor<Pothos::Object>()
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Object, unique))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Object, toString))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Object, getTypeString))
    .commit("Pothos/Object");
