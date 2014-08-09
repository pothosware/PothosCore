//
// Framework/CallRegistryImpl.hpp
//
// Template method implementations for CallRegistry.
//
// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Framework/CallRegistry.hpp>
#include <Pothos/Callable/CallableImpl.hpp>
#include <functional> //std::ref

namespace Pothos {

template <typename ReturnType, typename ClassType, typename InstanceType>
void CallRegistry::registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)())
{
    Callable call(method);
    call.bind(std::ref(*static_cast<ClassType *>(instance)), 0);
    this->registerCallable(name, call);
}

template <typename ReturnType, typename ClassType, typename InstanceType>
void CallRegistry::registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)() const)
{
    Callable call(method);
    call.bind(std::ref(*static_cast<ClassType *>(instance)), 0);
    this->registerCallable(name, call);
}

template <typename A0, typename ReturnType, typename ClassType, typename InstanceType>
void CallRegistry::registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0))
{
    Callable call(method);
    call.bind(std::ref(*static_cast<ClassType *>(instance)), 0);
    this->registerCallable(name, call);
}

template <typename A0, typename ReturnType, typename ClassType, typename InstanceType>
void CallRegistry::registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0) const)
{
    Callable call(method);
    call.bind(std::ref(*static_cast<ClassType *>(instance)), 0);
    this->registerCallable(name, call);
}

template <typename A0, typename A1, typename ReturnType, typename ClassType, typename InstanceType>
void CallRegistry::registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0, A1))
{
    Callable call(method);
    call.bind(std::ref(*static_cast<ClassType *>(instance)), 0);
    this->registerCallable(name, call);
}

template <typename A0, typename A1, typename ReturnType, typename ClassType, typename InstanceType>
void CallRegistry::registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0, A1) const)
{
    Callable call(method);
    call.bind(std::ref(*static_cast<ClassType *>(instance)), 0);
    this->registerCallable(name, call);
}

template <typename A0, typename A1, typename A2, typename ReturnType, typename ClassType, typename InstanceType>
void CallRegistry::registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2))
{
    Callable call(method);
    call.bind(std::ref(*static_cast<ClassType *>(instance)), 0);
    this->registerCallable(name, call);
}

template <typename A0, typename A1, typename A2, typename ReturnType, typename ClassType, typename InstanceType>
void CallRegistry::registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2) const)
{
    Callable call(method);
    call.bind(std::ref(*static_cast<ClassType *>(instance)), 0);
    this->registerCallable(name, call);
}

template <typename A0, typename A1, typename A2, typename A3, typename ReturnType, typename ClassType, typename InstanceType>
void CallRegistry::registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3))
{
    Callable call(method);
    call.bind(std::ref(*static_cast<ClassType *>(instance)), 0);
    this->registerCallable(name, call);
}

template <typename A0, typename A1, typename A2, typename A3, typename ReturnType, typename ClassType, typename InstanceType>
void CallRegistry::registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3) const)
{
    Callable call(method);
    call.bind(std::ref(*static_cast<ClassType *>(instance)), 0);
    this->registerCallable(name, call);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename ReturnType, typename ClassType, typename InstanceType>
void CallRegistry::registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4))
{
    Callable call(method);
    call.bind(std::ref(*static_cast<ClassType *>(instance)), 0);
    this->registerCallable(name, call);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename ReturnType, typename ClassType, typename InstanceType>
void CallRegistry::registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4) const)
{
    Callable call(method);
    call.bind(std::ref(*static_cast<ClassType *>(instance)), 0);
    this->registerCallable(name, call);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename ReturnType, typename ClassType, typename InstanceType>
void CallRegistry::registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4, A5))
{
    Callable call(method);
    call.bind(std::ref(*static_cast<ClassType *>(instance)), 0);
    this->registerCallable(name, call);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename ReturnType, typename ClassType, typename InstanceType>
void CallRegistry::registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4, A5) const)
{
    Callable call(method);
    call.bind(std::ref(*static_cast<ClassType *>(instance)), 0);
    this->registerCallable(name, call);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename ReturnType, typename ClassType, typename InstanceType>
void CallRegistry::registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4, A5, A6))
{
    Callable call(method);
    call.bind(std::ref(*static_cast<ClassType *>(instance)), 0);
    this->registerCallable(name, call);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename ReturnType, typename ClassType, typename InstanceType>
void CallRegistry::registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4, A5, A6) const)
{
    Callable call(method);
    call.bind(std::ref(*static_cast<ClassType *>(instance)), 0);
    this->registerCallable(name, call);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename ReturnType, typename ClassType, typename InstanceType>
void CallRegistry::registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4, A5, A6, A7))
{
    Callable call(method);
    call.bind(std::ref(*static_cast<ClassType *>(instance)), 0);
    this->registerCallable(name, call);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename ReturnType, typename ClassType, typename InstanceType>
void CallRegistry::registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4, A5, A6, A7) const)
{
    Callable call(method);
    call.bind(std::ref(*static_cast<ClassType *>(instance)), 0);
    this->registerCallable(name, call);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename ReturnType, typename ClassType, typename InstanceType>
void CallRegistry::registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4, A5, A6, A7, A8))
{
    Callable call(method);
    call.bind(std::ref(*static_cast<ClassType *>(instance)), 0);
    this->registerCallable(name, call);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename ReturnType, typename ClassType, typename InstanceType>
void CallRegistry::registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4, A5, A6, A7, A8) const)
{
    Callable call(method);
    call.bind(std::ref(*static_cast<ClassType *>(instance)), 0);
    this->registerCallable(name, call);
}


} //namespace Pothos
