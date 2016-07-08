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
template <typename A0, typename... Args>
void loadObjArgs(Object *out, A0 &&a0, Args&&... args)
{
    *out++ = Object(std::forward<A0>(a0));
    loadObjArgs(out, std::forward<Args>(args)...);
}

} //namespace Detail

template <typename ReturnType, typename... Args>
ReturnType CallInterface::call(Args&&... args) const
{
    Object r = this->callObject(std::forward<Args>(args)...);
    try
    {
        return r.convert<ReturnType>();
    }
    catch(const Exception &ex)
    {
        throw CallableReturnError("Pothos::Callable::call()", ex);
    }
}

template <typename... Args>
Object CallInterface::callObject(Args&&... args) const
{
    Object objArgs[sizeof...(args)];
    Detail::loadObjArgs(objArgs, std::forward<Args>(args)...);
    return this->opaqueCall(objArgs, sizeof...(args));
}

template <typename... Args>
void CallInterface::callVoid(Args&&... args) const
{
    this->callObject(std::forward<Args>(args)...);
}

} //namespace Pothos
