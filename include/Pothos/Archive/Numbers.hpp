///
/// \file Archive/Numbers.hpp
///
/// Numeric support for serialization.
/// Integer and floating point types.
///
/// \copyright
/// Copyright (c) 2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Archive/Archive.hpp>
#include <type_traits>

namespace Pothos {
namespace serialization {

template<typename Archive, typename T>
typename std::enable_if<std::is_integral<T>::value and sizeof(T) <= 4>::type
save(Archive &ar, const T &t, const unsigned int)
{
    ar.writeInt32(int(t));
}

template<typename Archive, typename T>
typename std::enable_if<std::is_integral<T>::value and sizeof(T) <= 4>::type
load(Archive &ar, T &t, const unsigned int)
{
    t = T(ar.readInt32());
}

template<typename Archive, typename T>
typename std::enable_if<std::is_integral<T>::value and sizeof(T) == 8>::type
save(Archive &ar, const T &t, const unsigned int)
{
    ar.writeInt64((long long)t);
}

template<typename Archive, typename T>
typename std::enable_if<std::is_integral<T>::value and sizeof(T) == 8>::type
load(Archive &ar, T &t, const unsigned int)
{
    t = T(ar.readInt64());
}

template <typename Archive, typename T>
typename std::enable_if<std::is_integral<T>::value>::type
serialize(Archive &ar, T &t, const unsigned int ver)
{
    Pothos::serialization::invoke_load_save(ar, t, ver);
}

}}
