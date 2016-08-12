///
/// \file Framework/BlockRegistryImpl.hpp
///
/// Inline member implementation for BlockRegistry class.
///
/// \copyright
/// Copyright (c) 2016-2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Framework/BlockRegistry.hpp>
#include <Pothos/Proxy.hpp>
#include <utility> //std::forward

template <typename... ArgsType>
Pothos::Proxy Pothos::BlockRegistry::make(const std::string &path, ArgsType&&... args)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");
    return registry.callProxy(path, std::forward<ArgsType>(args)...);
}
