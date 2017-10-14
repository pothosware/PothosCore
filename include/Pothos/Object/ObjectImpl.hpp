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
#include <type_traits> //std::conditional, std::decay
#include <functional> //std::reference_wrapper
#include <utility> //std::forward
#include <atomic>
#include <iosfwd>

namespace Pothos {
namespace Detail {

/***********************************************************************
 * special_decay_t = decay + unwrapping a reference wrapper
 * http://en.cppreference.com/w/cpp/utility/tuple/make_tuple
 **********************************************************************/
template <typename T>
struct unwrap_refwrapper
{
    using type = T;
};

template <typename T>
struct unwrap_refwrapper<std::reference_wrapper<T>>
{
    using type = T&;
};

template <typename T>
using special_decay_t = typename unwrap_refwrapper<typename std::decay<T>::type>::type;

/***********************************************************************
 * ObjectContainer interface
 **********************************************************************/
struct POTHOS_API ObjectContainer
{
    ObjectContainer(void);

    virtual ~ObjectContainer(void);

    virtual const std::type_info &type(void) const = 0;
    virtual const std::type_info &rawType(void) const = 0;

    std::atomic<int> counter;

    static void throwExtract(const Object &obj, const std::type_info &type);

    template <typename ValueType>
    static ValueType &extract(const Object &obj);

    virtual void *get(void) const = 0; //opaque pointer to internal value
};

/***********************************************************************
 * ObjectContainer templated subclass
 **********************************************************************/
template <typename ValueType>
struct ObjectContainerType
{
    typedef typename std::conditional<
        //is_copy_constructible doesnt seem to be standard,
        //since we just use this for iostream anyway,
        //use this conditional as a cheap replacement.
        std::is_base_of<std::ios_base, ValueType>::value,
        std::reference_wrapper<ValueType>, ValueType
    >::type type;
};

template <typename ValueType>
struct ObjectContainerT : ObjectContainer
{

    ObjectContainerT(void)
    {
        return;
    }

    template <typename T>
    ObjectContainerT(T &&value):
        value(std::forward<T>(value))
    {
        return;
    }

    ~ObjectContainerT(void)
    {
        return;
    }

    /*!
     * Remove the reference wrapper so the API can treat objects
     * passed in with std::ref as normal objects in the container.
     */
    const std::type_info &type(void) const
    {
        return typeid(special_decay_t<ValueType>);
    }

    /*!
     * Get the raw type so the implementation of extract can check
     * if this container is really a reference wrapper or not.
     */
    const std::type_info &rawType(void) const
    {
        return typeid(ValueType);
    }

    typename ObjectContainerType<ValueType>::type value;

    void *get(void) const
    {
        return (void *)std::addressof(this->value);
    }
};

/***********************************************************************
 * extract implementation with support for reference wrapper
 **********************************************************************/
template <typename ValueType>
ValueType &ObjectContainer::extract(const Object &obj)
{
    typedef typename std::decay<ValueType>::type DecayValueType;

    //Support for the special NullObject case when the _impl is nullptr:
    if (obj._impl == nullptr and obj.type() == typeid(ValueType))
    {
        return *(reinterpret_cast<typename ObjectContainerType<DecayValueType>::type *>(0));
    }

    //First check if the container is a reference wrapper of the ValueType.
    //Handle this special case so we can treat reference wrappers like normal.
    typedef std::reference_wrapper<DecayValueType> refWrapperType;
    if (obj._impl != nullptr and obj._impl->rawType() == typeid(refWrapperType))
    {
        return *(reinterpret_cast<typename ObjectContainerType<refWrapperType>::type *>(obj._impl->get()));
    }

    //Otherwise, check the type for a match and then extract the internal value
    if (obj._impl != nullptr and obj.type() == typeid(ValueType))
    {
        return *(reinterpret_cast<typename ObjectContainerType<DecayValueType>::type *>(obj._impl->get()));
    }

    Detail::ObjectContainer::throwExtract(obj, typeid(ValueType)); throw;
}

/***********************************************************************
 * template specialized factory for object containers
 **********************************************************************/
template <typename ValueType>
ObjectContainer *makeObjectContainer(ValueType &&value)
{
    return new ObjectContainerT<typename std::decay<ValueType>::type>(std::forward<ValueType>(value));
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
    Object newObj = this->convert(typeid(ValueType));
    return newObj.extract<ValueType>();
}

template <typename ValueType>
Object::operator ValueType(void) const
{
    return this->convert<ValueType>();
}

} //namespace Pothos
