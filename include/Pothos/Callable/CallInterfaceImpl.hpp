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

namespace Pothos {

namespace Detail {

//! The empty arguments tail case
inline void loadObjArgs(Object *)
{
    return;
}

//! Recurse to fill the output with each arg
template <typename Arg0Type, typename... ArgsType>
void loadObjArgs(Object *out, Arg0Type &&a0, ArgsType&&... args)
{
    *out++ = Object(std::forward<Arg0Type>(a0));
    loadObjArgs(out, std::forward<ArgsType>(args)...);
}

} //namespace Detail

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
    Object objArgs[sizeof...(args)];
    Detail::loadObjArgs(objArgs, std::forward<ArgsType>(args)...);
    return this->opaqueCall(objArgs, sizeof...(args));
}

template <typename... ArgsType>
void CallInterface::callVoid(ArgsType&&... args) const
{
    this->callObject(std::forward<ArgsType>(args)...);
}

} //namespace Pothos
