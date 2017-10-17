///
/// \file Object/ObjectImpl.hpp
///
/// Template implementation details for Object.
///
/// \copyright
/// Copyright (c) 2013-2017 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Object/Object.hpp>
#include <Pothos/Util/Templates.hpp> //special_decay_t
#include <type_traits> //std::decay
#include <utility> //std::forward
#include <atomic>

namespace Pothos {
namespace Detail {

/***********************************************************************
 * ObjectContainer interface
 **********************************************************************/
struct POTHOS_API ObjectContainer
{
    ObjectContainer(const std::type_info &);

    virtual ~ObjectContainer(void);

    void *internal; //!< Opaque pointer to internally held type

    const std::type_info &type; //!< Type info for internal type

    std::atomic<unsigned> counter; //! Atomic reference counter

    [[noreturn]] static void throwExtract(const Object &obj, const std::type_info &type);

    template <typename ValueType>
    static ValueType &extract(const Object &obj);
};

/***********************************************************************
 * ObjectContainer templated subclass
 **********************************************************************/
template <typename ValueType>
struct ObjectContainerT : ObjectContainer
{
    ObjectContainerT(void):
        ObjectContainer(typeid(ValueType))
    {
        internal = (void*)std::addressof(this->value);
    }

    template <typename T>
    ObjectContainerT(T &&value):
        ObjectContainer(typeid(ValueType)),
        value(std::forward<T>(value))
    {
        internal = (void*)std::addressof(this->value);
    }

    ~ObjectContainerT(void)
    {
        return;
    }

    ValueType value;
};

/***********************************************************************
 * extract implementation with support for reference wrapper
 **********************************************************************/
template <typename ValueType>
ValueType &ObjectContainer::extract(const Object &obj)
{
    typedef typename std::decay<ValueType>::type DecayValueType;

    //throw when the target type does not match the container type
    if (obj.type() != typeid(ValueType))
    {
        Detail::ObjectContainer::throwExtract(obj, typeid(ValueType));
    }

    //Support for the special NullObject case when the _impl is nullptr.
    //Otherwise, check the type for a match and then extract the internal value
    return *(reinterpret_cast<DecayValueType *>((obj._impl == nullptr)?0:obj._impl->internal));
}

/***********************************************************************
 * template specialized factory for object containers
 **********************************************************************/
template <typename ValueType>
ObjectContainer *makeObjectContainer(ValueType &&value)
{
    return new ObjectContainerT<Pothos::Util::special_decay_t<ValueType>>(std::forward<ValueType>(value));
}

/*!
 * Create an Object from a NullObject type.
 * The implementation always returns null.
 * We do not allocate for type NullObject.
 */
inline ObjectContainer *makeObjectContainer(NullObject &&)
{
    return nullptr;
}

/*!
 * Create an Object from a string char array.
 * This resulting object type will be std::string.
 * This is a convenience function to use null-terminated
 * char arrays without the explicit cast to std::string.
 */
POTHOS_API ObjectContainer *makeObjectContainer(const char *s);

} //namespace Detail

template <typename ValueType>
Object Object::make(ValueType &&value)
{
    Object o;
    o._impl = Detail::makeObjectContainer(std::forward<ValueType>(value));
    return o;
}

template <typename ValueType>
Object::Object(ValueType &&value):
    _impl(nullptr)
{
    _impl = Detail::makeObjectContainer(std::forward<ValueType>(value));
}

template <typename ValueType>
const ValueType &Object::extract(void) const
{
    return Detail::ObjectContainer::extract<ValueType>(*this);
}

template <typename ValueType>
ValueType Object::convert(void) const
{
    if (this->type() == typeid(ValueType)) return this->extract<ValueType>();
    Object newObj = this->convert(typeid(ValueType));
    return newObj.extract<ValueType>();
}

template <typename ValueType>
Object::operator ValueType(void) const
{
    return this->convert<ValueType>();
}

} //namespace Pothos
