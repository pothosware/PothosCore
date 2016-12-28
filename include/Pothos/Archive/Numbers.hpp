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
#include <Pothos/Archive/BinaryObject.hpp>
#include <type_traits>
#include <cfloat> //FLT_MANT_DIG, DBL_MANT_DIG
#include <cmath> //frexp, ldexp

//For the sake of portability, whatever type is size_t
//will always be packed and unpacked a 64-bit integer.
#include <cstddef> //size_t

namespace Pothos {
namespace serialization {

//------------ 32 bit integer support (and below) --------------//
template<typename Archive, typename T>
typename std::enable_if<std::is_integral<T>::value and
    (sizeof(T) <= 4 and !std::is_same<T, size_t>::value)>::type
save(Archive &ar, const T &t, const unsigned int)
{
    unsigned char buff[4];
    buff[0] = char(t >> 0);
    buff[1] = char(t >> 8);
    buff[2] = char(t >> 16);
    buff[3] = char(t >> 24);
    BinaryObject bo(buff, sizeof(buff));
    ar << bo;
}

template<typename Archive, typename T>
typename std::enable_if<std::is_integral<T>::value and
    (sizeof(T) <= 4 and !std::is_same<T, size_t>::value)>::type
load(Archive &ar, T &t, const unsigned int)
{
    unsigned char buff[4];
    BinaryObject bo(buff, sizeof(buff));
    ar >> bo;
    t = T(
        (static_cast<unsigned int>(buff[0]) << 0) |
        (static_cast<unsigned int>(buff[1]) << 8) |
        (static_cast<unsigned int>(buff[2]) << 16) |
        (static_cast<unsigned int>(buff[3]) << 24));
}

//------------ 64 bit integer support --------------//
template<typename Archive, typename T>
typename std::enable_if<std::is_integral<T>::value and
    (sizeof(T) == 8 or std::is_same<T, size_t>::value)>::type
save(Archive &ar, const T &t, const unsigned int)
{
    unsigned char buff[8];
    buff[0] = char(t >> 0);
    buff[1] = char(t >> 8);
    buff[2] = char(t >> 16);
    buff[3] = char(t >> 24);
    buff[4] = char(t >> 32);
    buff[5] = char(t >> 40);
    buff[6] = char(t >> 48);
    buff[7] = char(t >> 56);
    BinaryObject bo(buff, sizeof(buff));
    ar << bo;
}

template<typename Archive, typename T>
typename std::enable_if<std::is_integral<T>::value and
    (sizeof(T) == 8 or std::is_same<T, size_t>::value)>::type
load(Archive &ar, T &t, const unsigned int)
{
    unsigned char buff[8];
    BinaryObject bo(buff, sizeof(buff));
    ar >> bo;
    t = T(
        (static_cast<unsigned long long>(buff[0]) << 0) |
        (static_cast<unsigned long long>(buff[1]) << 8) |
        (static_cast<unsigned long long>(buff[2]) << 16) |
        (static_cast<unsigned long long>(buff[3]) << 24) |
        (static_cast<unsigned long long>(buff[4]) << 32) |
        (static_cast<unsigned long long>(buff[5]) << 40) |
        (static_cast<unsigned long long>(buff[6]) << 48) |
        (static_cast<unsigned long long>(buff[7]) << 56));
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
    Pothos::serialization::invokeSplit(ar, t, ver);
}

}}
