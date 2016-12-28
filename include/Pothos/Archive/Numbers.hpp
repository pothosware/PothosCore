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
#include <Pothos/Archive/Invoke.hpp>
#include <type_traits>
#include <cfloat> //FLT_MANT_DIG, DBL_MANT_DIG
#include <cmath> //frexp, ldexp

namespace Pothos {
namespace serialization {

//------------ 32 bit integer support (and below) --------------//
template<typename Archive, typename T>
typename std::enable_if<std::is_integral<T>::value and sizeof(T) <= 4>::type
save(Archive &ar, const T &t, const unsigned int)
{
    ar.writeInt32(static_cast<unsigned int>(t));
}

template<typename Archive, typename T>
typename std::enable_if<std::is_integral<T>::value and sizeof(T) <= 4>::type
load(Archive &ar, T &t, const unsigned int)
{
    t = T(ar.readInt32());
}

//------------ 64 bit integer support --------------//
template<typename Archive, typename T>
typename std::enable_if<std::is_integral<T>::value and sizeof(T) == 8>::type
save(Archive &ar, const T &t, const unsigned int)
{
    ar.writeInt64(static_cast<unsigned long long>(t));
}

template<typename Archive, typename T>
typename std::enable_if<std::is_integral<T>::value and sizeof(T) == 8>::type
load(Archive &ar, T &t, const unsigned int)
{
    t = T(ar.readInt64());
}

//------------ 32-bit float support --------------//
template<typename Archive>
void save(Archive &ar, const float &t, const unsigned int)
{
    int exp = 0;
    const float x = std::frexp(t, &exp);
    const auto man = (int)std::ldexp(x, FLT_MANT_DIG);
    ar & exp;
    ar & man;
}

template<typename Archive>
void load(Archive &ar, float &t, const unsigned int)
{
    int exp = 0;
    int man = 0;
    ar & exp;
    ar & man;
    t = std::ldexp(float(man), exp-FLT_MANT_DIG);
}

//------------ 64-bit float support --------------//
template<typename Archive>
void save(Archive &ar, const double &t, const unsigned int)
{
    int exp = 0;
    const double x = std::frexp(t, &exp);
    const auto man = (long long)std::ldexp(x, DBL_MANT_DIG);
    ar & exp;
    ar & man;
}

template<typename Archive>
void load(Archive &ar, double &t, const unsigned int)
{
    int exp = 0;
    long long man = 0;
    ar & exp;
    ar & man;
    t = std::ldexp(double(man), exp-DBL_MANT_DIG);
}

//------------ serialize for integers and floats --------------//
template <typename Archive, typename T>
typename std::enable_if<std::is_arithmetic<T>::value>::type
serialize(Archive &ar, T &t, const unsigned int ver)
{
    Pothos::serialization::invokeLoadSave(ar, t, ver);
}

}}
