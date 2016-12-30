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

namespace Pothos {
namespace serialization {

//------------ boolean support --------------//
template<typename Archive>
void save(Archive &ar, const bool &t, const unsigned int)
{
    unsigned char value(t?1:0);
    ar << value;
}

template<typename Archive>
void load(Archive &ar, bool &t, const unsigned int)
{
    unsigned char value;
    ar >> value;
    t = (value == 0)?false:true;
}

//------------ 8 bit integer support (all char types) --------------//
template<typename Archive, typename T>
typename std::enable_if<
    std::is_same<T, unsigned char>::value or
    std::is_same<T, signed char>::value or
    std::is_same<T, char>::value
>::type save(Archive &ar, const T &t, const unsigned int)
{
    BinaryObject bo(&t, 1);
    ar << bo;
}

template<typename Archive, typename T>
typename std::enable_if<
    std::is_same<T, unsigned char>::value or
    std::is_same<T, signed char>::value or
    std::is_same<T, char>::value
>::type load(Archive &ar, T &t, const unsigned int)
{
    BinaryObject bo(&t, 1);
    ar >> bo;
}

//------------ 16 bit integer support (short types) --------------//
template<typename Archive, typename T>
typename std::enable_if<
    std::is_same<T, unsigned short>::value or
    std::is_same<T, signed short>::value
>::type save(Archive &ar, const T &t, const unsigned int)
{
    unsigned char buff[2];
    buff[0] = static_cast<unsigned char>(t >> 0);
    buff[1] = static_cast<unsigned char>(t >> 8);
    BinaryObject bo(buff, sizeof(buff));
    ar << bo;
}

template<typename Archive, typename T>
typename std::enable_if<
    std::is_same<T, unsigned short>::value or
    std::is_same<T, signed short>::value
>::type load(Archive &ar, T &t, const unsigned int)
{
    unsigned char buff[2];
    BinaryObject bo(buff, sizeof(buff));
    ar >> bo;
    t = T(
        (static_cast<unsigned short>(buff[0]) << 0) |
        (static_cast<unsigned short>(buff[1]) << 8));
}

//------------ 32 bit integer support (int types) --------------//
template<typename Archive, typename T>
typename std::enable_if<
    std::is_same<T, unsigned int>::value or
    std::is_same<T, signed int>::value
>::type save(Archive &ar, const T &t, const unsigned int)
{
    unsigned char buff[4];
    buff[0] = static_cast<unsigned char>(t >> 0);
    buff[1] = static_cast<unsigned char>(t >> 8);
    buff[2] = static_cast<unsigned char>(t >> 16);
    buff[3] = static_cast<unsigned char>(t >> 24);
    BinaryObject bo(buff, sizeof(buff));
    ar << bo;
}

template<typename Archive, typename T>
typename std::enable_if<
    std::is_same<T, unsigned int>::value or
    std::is_same<T, signed int>::value
>::type load(Archive &ar, T &t, const unsigned int)
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

//------------ 64 bit integer support (long types)--------------//
// the size of long notoriously varies among platforms
// always serialize long with 8 bytes for portability
template<typename Archive, typename T>
typename std::enable_if<
    std::is_same<T, unsigned long>::value or
    std::is_same<T, signed long>::value or
    std::is_same<T, unsigned long long>::value or
    std::is_same<T, signed long long>::value
>::type save(Archive &ar, const T &t, const unsigned int)
{
    unsigned char buff[8];
    buff[0] = static_cast<unsigned char>(t >> 0);
    buff[1] = static_cast<unsigned char>(t >> 8);
    buff[2] = static_cast<unsigned char>(t >> 16);
    buff[3] = static_cast<unsigned char>(t >> 24);
    buff[4] = static_cast<unsigned char>(t >> 32);
    buff[5] = static_cast<unsigned char>(t >> 40);
    buff[6] = static_cast<unsigned char>(t >> 48);
    buff[7] = static_cast<unsigned char>(t >> 56);
    BinaryObject bo(buff, sizeof(buff));
    ar << bo;
}

template<typename Archive, typename T>
typename std::enable_if<
    std::is_same<T, unsigned long>::value or
    std::is_same<T, signed long>::value or
    std::is_same<T, unsigned long long>::value or
    std::is_same<T, signed long long>::value
>::type load(Archive &ar, T &t, const unsigned int)
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
