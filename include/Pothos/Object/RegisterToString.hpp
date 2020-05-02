///
/// \file Object/RegisterToString.hpp
///
/// A convenience function to register functions to be used with
/// Object::toString().
///
/// \copyright
/// Copyright (c) 2020 Nicholas Corgan
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once

#include <Pothos/Callable.hpp>
#include <Pothos/Exception.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/Util/Templates.hpp>
#include <Pothos/Util/TypeInfo.hpp>

#include <functional>
#include <memory>
#include <string>

namespace Pothos
{

template <typename T>
using ToStringFunc = std::function<std::string(const T&)>;

template <typename T>
static void registerToStringFunc(
    const std::string& typeName,
    const ToStringFunc<T>& toStringFunc,
    bool registerPointerTypes);

//
// Implementation
//

template <typename T>
void registerToStringFunc(
    const std::string& pluginSubtree,
    const ToStringFunc<T>& toStringFunc,
    bool registerPointerTypes)
{
    // Store our own static copy so the lambdas below don't try to use
    // the reference parameter after the underlying std::function is
    // deleted.
    static const ToStringFunc<T> func(toStringFunc);

    const std::string prefix = "/object/tostring";
    Pothos::PluginRegistry::addCall(
        prefix+"/"+pluginSubtree,
        func);

    if(registerPointerTypes)
    {
        const std::function<std::string(T*)> ptrToString([&](T* ptr)
        {
            if(nullptr == ptr) return "Null pointer: " + Pothos::Util::typeInfoToString(typeid(T));
            else               return "Pointer: " + func(*ptr);
        });
        const std::function<std::string(const T*)> constPtrToString([&](const T* ptr)
        {
            if(nullptr == ptr) return "Null pointer: " + Pothos::Util::typeInfoToString(typeid(T));
            else               return "Pointer: " + func(*ptr);
        });
        const std::function<std::string(const std::shared_ptr<T>&)> sptrToString([&](const std::shared_ptr<T>& sptr)
        {
            if(nullptr == sptr.get()) return "Null shared pointer: " + Pothos::Util::typeInfoToString(typeid(T));
            else                      return "Shared pointer: " + func(*sptr);
        });

        Pothos::PluginRegistry::addCall(
            prefix+"/"+pluginSubtree+"_ptr",
            Pothos::Callable(ptrToString));
        Pothos::PluginRegistry::addCall(
            prefix+"/"+pluginSubtree+"_const_ptr",
            Pothos::Callable(constPtrToString));
        Pothos::PluginRegistry::addCall(
            prefix+"/"+pluginSubtree+"_sptr",
            Pothos::Callable(sptrToString));
    }
}

}
