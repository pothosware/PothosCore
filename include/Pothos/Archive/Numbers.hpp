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
#include <Pothos/Archive/Macros.hpp>
#include <string>
#include <type_traits>

namespace Pothos {
namespace serialization {

template<typename Archive, typename T>
typename std::enable_if<std::is_integral<T>::value and sizeof(T) <= 4>::type
save(Archive &ar, const T &t, const unsigned int)
{
    ar.writeInt32(t);
}

template<typename Archive, typename T>
typename std::enable_if<std::is_integral<T>::value and sizeof(T) <= 4>::type
load(Archive &ar, T &t, const unsigned int)
{
    t = ar.readInt32();
}

template<typename Archive, typename T>
typename std::enable_if<std::is_integral<T>::value and sizeof(T) == 8>::type
save(Archive &ar, const T &t, const unsigned int)
{
    ar.writeInt64(t);
}

template<typename Archive, typename T>
typename std::enable_if<std::is_integral<T>::value and sizeof(T) == 8>::type
load(Archive &ar, T &t, const unsigned int)
{
    t = ar.readInt64();
}

}}

POTHOS_SERIALIZATION_SPLIT_FREE(bool)
POTHOS_SERIALIZATION_SPLIT_FREE(char)
POTHOS_SERIALIZATION_SPLIT_FREE(signed char)
POTHOS_SERIALIZATION_SPLIT_FREE(unsigned char)
POTHOS_SERIALIZATION_SPLIT_FREE(signed short)
POTHOS_SERIALIZATION_SPLIT_FREE(unsigned short)
POTHOS_SERIALIZATION_SPLIT_FREE(signed int)
POTHOS_SERIALIZATION_SPLIT_FREE(unsigned int)
POTHOS_SERIALIZATION_SPLIT_FREE(signed long)
POTHOS_SERIALIZATION_SPLIT_FREE(unsigned long)
POTHOS_SERIALIZATION_SPLIT_FREE(signed long long)
POTHOS_SERIALIZATION_SPLIT_FREE(unsigned long long)

POTHOS_CLASS_EXPORT(bool)
POTHOS_CLASS_EXPORT(char)
POTHOS_CLASS_EXPORT(signed char)
POTHOS_CLASS_EXPORT(unsigned char)
POTHOS_CLASS_EXPORT(signed short)
POTHOS_CLASS_EXPORT(unsigned short)
POTHOS_CLASS_EXPORT(signed int)
POTHOS_CLASS_EXPORT(unsigned int)
POTHOS_CLASS_EXPORT(signed long)
POTHOS_CLASS_EXPORT(unsigned long)
POTHOS_CLASS_EXPORT(signed long long)
POTHOS_CLASS_EXPORT(unsigned long long)
