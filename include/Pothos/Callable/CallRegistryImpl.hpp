///
/// \file Callable/CallRegistryImpl.hpp
///
/// Template method implementations for CallRegistry.
///
/// \copyright
/// Copyright (c) 2014-2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Callable/CallRegistry.hpp>
#include <Pothos/Callable/CallableImpl.hpp>
#include <functional> //std::ref

namespace Pothos {

template <typename... ArgsType, typename ReturnType, typename ClassType, typename InstanceType>
void CallRegistry::registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(ArgsType...))
{
    Callable call(method);
    call.bind(std::ref(*static_cast<ClassType *>(instance)), 0);
    this->registerCallable(name, call);
}

template <typename... ArgsType, typename ReturnType, typename ClassType, typename InstanceType>
void CallRegistry::registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(ArgsType...) const)
{
    Callable call(method);
    call.bind(std::ref(*static_cast<ClassType *>(instance)), 0);
    this->registerCallable(name, call);
}

} //namespace Pothos
