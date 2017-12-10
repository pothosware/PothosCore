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
#include <Pothos/Object/Exception.hpp>
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
};

/***********************************************************************
 * ObjectContainer templated subclass
 **********************************************************************/
template <typename ValueType>
struct ObjectContainerT : ObjectContainer
{
    template <typename... Args>
    ObjectContainerT(Args&&... args):
        ObjectContainer(typeid(ValueType)),
        value(std::forward<Args>(args)...)
    {
        internal = (void*)std::addressof(this->value);
    }

    ~ObjectContainerT(void)
    {
        return;
    }

    ValueType value;
};

template <typename ValueType, typename... Args>
typename std::enable_if<!std::is_same<NullObject, ValueType>::value, ObjectContainer *>::type
makeObjectContainer(Args&&... args)
{
    return new ObjectContainerT<Pothos::Util::special_decay_t<ValueType>>(std::forward<Args>(args)...);
}

template <typename ValueType, typename... Args>
typename std::enable_if<std::is_same<NullObject, ValueType>::value, ObjectContainer *>::type
makeObjectContainer(Args&&...)
{
    return nullptr;
}

/***********************************************************************
 * extract implementation: either null type or direct pointer cast
 **********************************************************************/
template <typename ValueType>
ValueType &extractObject(const Object &obj)
{
    //Support for the special NullObject case when the _impl is nullptr.
    //Otherwise, check the type for a match and then extract the internal value
    typedef typename std::decay<ValueType>::type DecayValueType;
    return *(reinterpret_cast<DecayValueType *>((obj._impl == nullptr)?0:obj._impl->internal));
}

[[noreturn]] POTHOS_API void throwExtract(const Object &obj, const std::type_info &type);

/***********************************************************************
 * convertObject either converts a object to a desired type
 * or returns a object if the requested type was a object
 **********************************************************************/
template <typename T>
typename std::enable_if<!std::is_same<T, Object>::value, T>::type
convertObject(const Object &obj)
{
    return extractObject<T>(obj.convert(typeid(T)));
}

template <typename T>
typename std::enable_if<std::is_same<T, Object>::value, T>::type
convertObject(const Object &obj)
{
    return obj;
}

} //namespace Detail

template <typename ValueType>
Object Object::make(ValueType &&value)
{
    return Object(InPlace<ValueType>(), std::forward<ValueType>(value));
}

template <typename ValueType, typename... Args>
Object Object::make(Args&&... args)
{
    return Object(InPlace<ValueType>(), std::forward<Args>(args)...);
}

template <typename ValueType, typename>
Object::Object(ValueType &&value):
    _impl(Detail::makeObjectContainer<ValueType>(std::forward<ValueType>(value)))
{
    return;
}

template <typename ValueType, typename... Args>
Object::Object(InPlace<ValueType>, Args&&... args):
    _impl(Detail::makeObjectContainer<ValueType>(std::forward<Args>(args)...))
{
    return;
}

inline Object::Object(const char *s):
    _impl(Detail::makeObjectContainer<std::string>(s))
{
    return;
}

inline const std::type_info &Pothos::Object::type(void) const
{
    return (_impl == nullptr)?typeid(NullObject):_impl->type;
}

template <typename ValueType>
const ValueType &Object::extract(void) const
{
    if (this->type() == typeid(ValueType))
    {
        return Detail::extractObject<ValueType>(*this);
    }
    Detail::throwExtract(*this, typeid(ValueType));
}

template <typename ValueType>
ValueType &Object::ref(void)
{
    if (not this->unique()) throw ObjectConvertError("Pothos::Object::ref()",
        "Multiple object ownership, access denied to non-const reference");
    return const_cast<ValueType &>(this->extract<ValueType>());
}

template <typename ValueType>
ValueType Object::convert(void) const
{
    if (this->type() == typeid(ValueType))
    {
        return Detail::extractObject<ValueType>(*this);
    }
    return Detail::convertObject<ValueType>(*this);
}

template <typename ValueType>
Object::operator ValueType(void) const
{
    return this->convert<ValueType>();
}

} //namespace Pothos
