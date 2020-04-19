// Copyright (c) 2020 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once

#include <Pothos/Object.hpp>
#include <Pothos/Util/TypeInfo.hpp>

#include <memory>
#include <string>

template <typename T>
static std::string ptrObjectToString(T* ptr)
{
    if(nullptr == ptr) return "Null pointer to " + Pothos::Util::typeInfoToString(typeid(T));
    else return "Pointer to " + Pothos::Object(std::ref(*ptr)).toString();
}

template <typename T>
static std::string sptrObjectToString(typename std::shared_ptr<T> sptr)
{
    if(nullptr == sptr.get()) return "Null shared pointer to " + Pothos::Util::typeInfoToString(typeid(T));
    else return "Shared pointer to " + Pothos::Object(std::ref(*sptr)).toString();
}
