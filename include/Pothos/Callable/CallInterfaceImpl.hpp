///
/// \file Callable/CallInterfaceImpl.hpp
///
/// Template implementation details for CallInterface.
///
/// \copyright
/// Copyright (c) 2013-2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Callable/CallInterface.hpp>
#include <Pothos/Callable/Exception.hpp>
#include <Pothos/Object/ObjectImpl.hpp>
#include <utility> //std::forward
#include <array>

namespace Pothos {

template <typename ReturnType, typename... ArgsType>
ReturnType CallInterface::call(ArgsType&&... args) const
{
    Object r = this->callObject(std::forward<ArgsType>(args)...);
    try
    {
        return r.convert<ReturnType>();
    }
    catch(const Exception &ex)
    {
        throw CallableReturnError("Pothos::Callable::call()", ex);
    }
}

template <typename... ArgsType>
Object CallInterface::callObject(ArgsType&&... args) const
{
    const std::array<Object, sizeof...(ArgsType)> objArgs{Object(std::forward<ArgsType>(args))...};
    return this->opaqueCall(objArgs.data(), sizeof...(args));
}

template <typename... ArgsType>
void CallInterface::callVoid(ArgsType&&... args) const
{
    this->callObject(std::forward<ArgsType>(args)...);
}

} //namespace Pothos
