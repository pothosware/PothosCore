//
// Managed/ClassImpl.hpp
//
// Template implementation details for ManagedClass.
//
// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Callable/CallableImpl.hpp>
#include <Pothos/Managed/Class.hpp>
#include <functional> //std::reference_wrapper

namespace Pothos {

namespace Detail {

template <typename T>
std::reference_wrapper<T> referenceToWrapper(T &v)
{
    return std::ref(v);
}

template <typename T>
std::reference_wrapper<T> pointerToWrapper(T *v)
{
    return std::ref(*v);
}

template <typename T>
std::reference_wrapper<T> sharedToWrapper(std::shared_ptr<T> &v)
{
    return std::ref(*v);
}

template <typename T>
void deleteValue(T &v)
{
    delete (&v);
}

template <typename ClassType, typename ValueType>
static const ValueType &getField(ClassType &i, const std::function<ValueType &(ClassType *)> &getRef)
{
    return getRef(&i);
}

template <typename ClassType, typename ValueType>
static void setField(ClassType &i, const std::function<ValueType &(ClassType *)> &getRef, const ValueType &v)
{
    getRef(&i) = v;
}

} //namespace Detail

template <typename ClassType>
ManagedClass &ManagedClass::registerClass(void)
{
    if (not getReferenceToWrapper() or not getPointerToWrapper() or not getSharedToWrapper())
    {
        registerReferenceToWrapper(Callable(&Detail::referenceToWrapper<ClassType>));
        registerPointerToWrapper(Callable(&Detail::pointerToWrapper<ClassType>));
        registerSharedToWrapper(Callable(&Detail::sharedToWrapper<ClassType>));
        registerMethod("delete", Callable(&Detail::deleteValue<ClassType>));
    }
    return *this;
}

template <typename ClassType, typename ValueType>
ManagedClass &ManagedClass::registerField(const std::string &name, ValueType ClassType::*member)
{
    std::function<ValueType &(ClassType *)> getRef = std::mem_fn(member);
    this->registerMethod("get:"+name, Callable(&Detail::getField<ClassType, ValueType>).bind(getRef, 1));
    this->registerMethod("set:"+name, Callable(&Detail::setField<ClassType, ValueType>).bind(getRef, 1));
    return *this;
}

template <typename ClassType>
ManagedClass &ManagedClass::registerConstructor(void)
{
    this->registerClass<ClassType>();
    this->registerConstructor(Callable::factory<ClassType>());
    this->registerStaticMethod("new", Callable::factoryNew<ClassType>());
    this->registerStaticMethod("shared", Callable::factoryShared<ClassType>());
    return *this;
}

template <typename ReturnType>
ManagedClass &ManagedClass::registerStaticMethod(const std::string &name, ReturnType(*method)())
{
    this->registerStaticMethod(name, Callable(method));
    return *this;
}

template <typename ReturnType, typename ClassType>
ManagedClass &ManagedClass::registerMethod(const std::string &name, ReturnType(ClassType::*method)())
{
    this->registerClass<ClassType>();
    this->registerMethod(name, Callable(method));
    return *this;
}

template <typename ReturnType, typename ClassType>
ManagedClass &ManagedClass::registerMethod(const std::string &name, ReturnType(ClassType::*method)() const)
{
    this->registerClass<ClassType>();
    this->registerMethod(name, Callable(method));
    return *this;
}

template <typename ClassType, typename A0>
ManagedClass &ManagedClass::registerConstructor(void)
{
    this->registerClass<ClassType>();
    this->registerConstructor(Callable::factory<ClassType, A0>());
    this->registerStaticMethod("new", Callable::factoryNew<ClassType, A0>());
    this->registerStaticMethod("shared", Callable::factoryShared<ClassType, A0>());
    return *this;
}

template <typename A0, typename ReturnType>
ManagedClass &ManagedClass::registerStaticMethod(const std::string &name, ReturnType(*method)(A0))
{
    this->registerStaticMethod(name, Callable(method));
    return *this;
}

template <typename A0, typename ReturnType, typename ClassType>
ManagedClass &ManagedClass::registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0))
{
    this->registerClass<ClassType>();
    this->registerMethod(name, Callable(method));
    return *this;
}

template <typename A0, typename ReturnType, typename ClassType>
ManagedClass &ManagedClass::registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0) const)
{
    this->registerClass<ClassType>();
    this->registerMethod(name, Callable(method));
    return *this;
}

template <typename ClassType, typename A0, typename A1>
ManagedClass &ManagedClass::registerConstructor(void)
{
    this->registerClass<ClassType>();
    this->registerConstructor(Callable::factory<ClassType, A0, A1>());
    this->registerStaticMethod("new", Callable::factoryNew<ClassType, A0, A1>());
    this->registerStaticMethod("shared", Callable::factoryShared<ClassType, A0, A1>());
    return *this;
}

template <typename A0, typename A1, typename ReturnType>
ManagedClass &ManagedClass::registerStaticMethod(const std::string &name, ReturnType(*method)(A0, A1))
{
    this->registerStaticMethod(name, Callable(method));
    return *this;
}

template <typename A0, typename A1, typename ReturnType, typename ClassType>
ManagedClass &ManagedClass::registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0, A1))
{
    this->registerClass<ClassType>();
    this->registerMethod(name, Callable(method));
    return *this;
}

template <typename A0, typename A1, typename ReturnType, typename ClassType>
ManagedClass &ManagedClass::registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0, A1) const)
{
    this->registerClass<ClassType>();
    this->registerMethod(name, Callable(method));
    return *this;
}

template <typename ClassType, typename A0, typename A1, typename A2>
ManagedClass &ManagedClass::registerConstructor(void)
{
    this->registerClass<ClassType>();
    this->registerConstructor(Callable::factory<ClassType, A0, A1, A2>());
    this->registerStaticMethod("new", Callable::factoryNew<ClassType, A0, A1, A2>());
    this->registerStaticMethod("shared", Callable::factoryShared<ClassType, A0, A1, A2>());
    return *this;
}

template <typename A0, typename A1, typename A2, typename ReturnType>
ManagedClass &ManagedClass::registerStaticMethod(const std::string &name, ReturnType(*method)(A0, A1, A2))
{
    this->registerStaticMethod(name, Callable(method));
    return *this;
}

template <typename A0, typename A1, typename A2, typename ReturnType, typename ClassType>
ManagedClass &ManagedClass::registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2))
{
    this->registerClass<ClassType>();
    this->registerMethod(name, Callable(method));
    return *this;
}

template <typename A0, typename A1, typename A2, typename ReturnType, typename ClassType>
ManagedClass &ManagedClass::registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2) const)
{
    this->registerClass<ClassType>();
    this->registerMethod(name, Callable(method));
    return *this;
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3>
ManagedClass &ManagedClass::registerConstructor(void)
{
    this->registerClass<ClassType>();
    this->registerConstructor(Callable::factory<ClassType, A0, A1, A2, A3>());
    this->registerStaticMethod("new", Callable::factoryNew<ClassType, A0, A1, A2, A3>());
    this->registerStaticMethod("shared", Callable::factoryShared<ClassType, A0, A1, A2, A3>());
    return *this;
}

template <typename A0, typename A1, typename A2, typename A3, typename ReturnType>
ManagedClass &ManagedClass::registerStaticMethod(const std::string &name, ReturnType(*method)(A0, A1, A2, A3))
{
    this->registerStaticMethod(name, Callable(method));
    return *this;
}

template <typename A0, typename A1, typename A2, typename A3, typename ReturnType, typename ClassType>
ManagedClass &ManagedClass::registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3))
{
    this->registerClass<ClassType>();
    this->registerMethod(name, Callable(method));
    return *this;
}

template <typename A0, typename A1, typename A2, typename A3, typename ReturnType, typename ClassType>
ManagedClass &ManagedClass::registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3) const)
{
    this->registerClass<ClassType>();
    this->registerMethod(name, Callable(method));
    return *this;
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4>
ManagedClass &ManagedClass::registerConstructor(void)
{
    this->registerClass<ClassType>();
    this->registerConstructor(Callable::factory<ClassType, A0, A1, A2, A3, A4>());
    this->registerStaticMethod("new", Callable::factoryNew<ClassType, A0, A1, A2, A3, A4>());
    this->registerStaticMethod("shared", Callable::factoryShared<ClassType, A0, A1, A2, A3, A4>());
    return *this;
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename ReturnType>
ManagedClass &ManagedClass::registerStaticMethod(const std::string &name, ReturnType(*method)(A0, A1, A2, A3, A4))
{
    this->registerStaticMethod(name, Callable(method));
    return *this;
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename ReturnType, typename ClassType>
ManagedClass &ManagedClass::registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4))
{
    this->registerClass<ClassType>();
    this->registerMethod(name, Callable(method));
    return *this;
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename ReturnType, typename ClassType>
ManagedClass &ManagedClass::registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4) const)
{
    this->registerClass<ClassType>();
    this->registerMethod(name, Callable(method));
    return *this;
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
ManagedClass &ManagedClass::registerConstructor(void)
{
    this->registerClass<ClassType>();
    this->registerConstructor(Callable::factory<ClassType, A0, A1, A2, A3, A4, A5>());
    this->registerStaticMethod("new", Callable::factoryNew<ClassType, A0, A1, A2, A3, A4, A5>());
    this->registerStaticMethod("shared", Callable::factoryShared<ClassType, A0, A1, A2, A3, A4, A5>());
    return *this;
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename ReturnType>
ManagedClass &ManagedClass::registerStaticMethod(const std::string &name, ReturnType(*method)(A0, A1, A2, A3, A4, A5))
{
    this->registerStaticMethod(name, Callable(method));
    return *this;
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename ReturnType, typename ClassType>
ManagedClass &ManagedClass::registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4, A5))
{
    this->registerClass<ClassType>();
    this->registerMethod(name, Callable(method));
    return *this;
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename ReturnType, typename ClassType>
ManagedClass &ManagedClass::registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4, A5) const)
{
    this->registerClass<ClassType>();
    this->registerMethod(name, Callable(method));
    return *this;
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
ManagedClass &ManagedClass::registerConstructor(void)
{
    this->registerClass<ClassType>();
    this->registerConstructor(Callable::factory<ClassType, A0, A1, A2, A3, A4, A5, A6>());
    this->registerStaticMethod("new", Callable::factoryNew<ClassType, A0, A1, A2, A3, A4, A5, A6>());
    this->registerStaticMethod("shared", Callable::factoryShared<ClassType, A0, A1, A2, A3, A4, A5, A6>());
    return *this;
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename ReturnType>
ManagedClass &ManagedClass::registerStaticMethod(const std::string &name, ReturnType(*method)(A0, A1, A2, A3, A4, A5, A6))
{
    this->registerStaticMethod(name, Callable(method));
    return *this;
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename ReturnType, typename ClassType>
ManagedClass &ManagedClass::registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4, A5, A6))
{
    this->registerClass<ClassType>();
    this->registerMethod(name, Callable(method));
    return *this;
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename ReturnType, typename ClassType>
ManagedClass &ManagedClass::registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4, A5, A6) const)
{
    this->registerClass<ClassType>();
    this->registerMethod(name, Callable(method));
    return *this;
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
ManagedClass &ManagedClass::registerConstructor(void)
{
    this->registerClass<ClassType>();
    this->registerConstructor(Callable::factory<ClassType, A0, A1, A2, A3, A4, A5, A6, A7>());
    this->registerStaticMethod("new", Callable::factoryNew<ClassType, A0, A1, A2, A3, A4, A5, A6, A7>());
    this->registerStaticMethod("shared", Callable::factoryShared<ClassType, A0, A1, A2, A3, A4, A5, A6, A7>());
    return *this;
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename ReturnType>
ManagedClass &ManagedClass::registerStaticMethod(const std::string &name, ReturnType(*method)(A0, A1, A2, A3, A4, A5, A6, A7))
{
    this->registerStaticMethod(name, Callable(method));
    return *this;
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename ReturnType, typename ClassType>
ManagedClass &ManagedClass::registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4, A5, A6, A7))
{
    this->registerClass<ClassType>();
    this->registerMethod(name, Callable(method));
    return *this;
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename ReturnType, typename ClassType>
ManagedClass &ManagedClass::registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4, A5, A6, A7) const)
{
    this->registerClass<ClassType>();
    this->registerMethod(name, Callable(method));
    return *this;
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
ManagedClass &ManagedClass::registerConstructor(void)
{
    this->registerClass<ClassType>();
    this->registerConstructor(Callable::factory<ClassType, A0, A1, A2, A3, A4, A5, A6, A7, A8>());
    this->registerStaticMethod("new", Callable::factoryNew<ClassType, A0, A1, A2, A3, A4, A5, A6, A7, A8>());
    this->registerStaticMethod("shared", Callable::factoryShared<ClassType, A0, A1, A2, A3, A4, A5, A6, A7, A8>());
    return *this;
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename ReturnType>
ManagedClass &ManagedClass::registerStaticMethod(const std::string &name, ReturnType(*method)(A0, A1, A2, A3, A4, A5, A6, A7, A8))
{
    this->registerStaticMethod(name, Callable(method));
    return *this;
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename ReturnType, typename ClassType>
ManagedClass &ManagedClass::registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4, A5, A6, A7, A8))
{
    this->registerClass<ClassType>();
    this->registerMethod(name, Callable(method));
    return *this;
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename ReturnType, typename ClassType>
ManagedClass &ManagedClass::registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4, A5, A6, A7, A8) const)
{
    this->registerClass<ClassType>();
    this->registerMethod(name, Callable(method));
    return *this;
}


template <typename ClassType>
ManagedClass &ManagedClass::registerOpaqueConstructor(void)
{
    this->registerClass<ClassType>();
    this->registerOpaqueConstructor(Callable::factory<ClassType, const Object *, const size_t>());
    return *this;
}

} //namespace Pothos
