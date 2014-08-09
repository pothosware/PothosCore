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

#for $NARGS in range($MAX_ARGS)
template <$expand('typename A%d', $NARGS), typename ReturnType, typename ClassType, typename InstanceType>
void CallRegistry::registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)($expand('A%d', $NARGS)))
{
    Callable call(method);
    call.bind(std::ref(*static_cast<ClassType *>(instance)), 0);
    this->registerCallable(name, call);
}

template <$expand('typename A%d', $NARGS), typename ReturnType, typename ClassType, typename InstanceType>
void CallRegistry::registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)($expand('A%d', $NARGS)) const)
{
    Callable call(method);
    call.bind(std::ref(*static_cast<ClassType *>(instance)), 0);
    this->registerCallable(name, call);
}

#end for

} //namespace Pothos
