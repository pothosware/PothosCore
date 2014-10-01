///
/// \file Object/ObjectImpl.hpp
///
/// Template implementation details for Object.
///
/// \copyright
/// Copyright (c) 2013-2014 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Object/Object.hpp>
#include <type_traits> //std::conditional, std::decay
#include <functional> //std::reference_wrapper
#include <cstdlib> //size_t
#include <utility> //std::forward
#include <atomic>
#include <iosfwd>

namespace Pothos {
namespace Detail {

/***********************************************************************
 * meta-template foo to implement remove_reference_wrapper
 * http://en.wikipedia.org/wiki/Substitution_failure_is_not_an_error
 **********************************************************************/
template <typename T, bool C> struct extract_type_helper;
template <typename T> struct extract_type_helper<T, true>
{
    typedef typename T::type type;
};

template <typename T> struct extract_type_helper<T, false>
{
    typedef T type;
};

template <typename T>
class remove_reference_wrapper
{
private:
    // Types "yes" and "no" are guaranteed to have different sizes
    typedef char yes[1];
    typedef char no[2];

    template <typename C>
    static yes& test(typename C::type*);

    template <typename C>
    static no& test(C *);

    static const bool has_type_field = sizeof(test<T>(nullptr)) == sizeof(yes);
    typedef typename extract_type_helper<T, has_type_field>::type extracted_type;
    typedef std::is_same<std::reference_wrapper<extracted_type>, T> is_reference_wrapper;

public:
    typedef typename std::conditional<is_reference_wrapper::value, extracted_type, T>::type type;
};

/***********************************************************************
 * ObjectContainer interface
 **********************************************************************/
struct POTHOS_API ObjectContainer
{
    ObjectContainer(void);

    virtual ~ObjectContainer(void);

    void *operator new(const size_t size);

    void operator delete(void *memory, const size_t size);

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
        return typeid(typename remove_reference_wrapper<ValueType>::type);
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

} //namespace Pothos
