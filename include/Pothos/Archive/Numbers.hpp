///
/// \file Archive/Numbers.hpp
///
/// Numeric support for serialization.
///
/// Integers are serialized into a portable byte ordering
/// to work across big and little platform endianess.
/// In addition, all long types get serialized as 8 bytes
/// because the size of long may vary from 4 to 8 bytes.
///
/// Floats are cast to an integer type of identical size
/// and serialized as an integer; also for platform endianess.
/// The implementation assumes the IEEE-754 format for floats,
/// but could easily be expanded for soft packing of IEEE-754.
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
#include <limits> //is_iec559

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
    const auto v = static_cast<unsigned short>(t);
    buff[0] = static_cast<unsigned char>(v >> 0);
    buff[1] = static_cast<unsigned char>(v >> 8);
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
    t = static_cast<T>(
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
    const auto v = static_cast<unsigned int>(t);
    buff[0] = static_cast<unsigned char>(v >> 0);
    buff[1] = static_cast<unsigned char>(v >> 8);
    buff[2] = static_cast<unsigned char>(v >> 16);
    buff[3] = static_cast<unsigned char>(v >> 24);
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
    t = static_cast<T>(
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
    const auto v = static_cast<unsigned long long>(t);
    buff[0] = static_cast<unsigned char>(v >> 0);
    buff[1] = static_cast<unsigned char>(v >> 8);
    buff[2] = static_cast<unsigned char>(v >> 16);
    buff[3] = static_cast<unsigned char>(v >> 24);
    buff[4] = static_cast<unsigned char>(v >> 32);
    buff[5] = static_cast<unsigned char>(v >> 40);
    buff[6] = static_cast<unsigned char>(v >> 48);
    buff[7] = static_cast<unsigned char>(v >> 56);
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
    t = static_cast<T>(
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
static_assert(std::numeric_limits<float>::is_iec559, "System is IEEE-754");

template<typename Archive>
typename std::enable_if<std::numeric_limits<float>::is_iec559>::type
save(Archive &ar, const float &t, const unsigned int)
{
    const void *bin(&t);
    ar << *static_cast<const unsigned int *>(bin);
}

template<typename Archive>
typename std::enable_if<std::numeric_limits<float>::is_iec559>::type
load(Archive &ar, float &t, const unsigned int)
{
    unsigned int num;
    ar >> num;
    const void *bin(&num);
    t = *reinterpret_cast<const float *>(bin);
}

//------------ 64-bit float support --------------//
static_assert(std::numeric_limits<double>::is_iec559, "System is IEEE-754");

template<typename Archive>
typename std::enable_if<std::numeric_limits<double>::is_iec559>::type
save(Archive &ar, const double &t, const unsigned int)
{
    const void *bin(&t);
    ar << *static_cast<const unsigned long long *>(bin);
}

template<typename Archive>
typename std::enable_if<std::numeric_limits<double>::is_iec559>::type
load(Archive &ar, double &t, const unsigned int)
{
    unsigned long long num;
    ar >> num;
    const void *bin(&num);
    t = *reinterpret_cast<const double *>(bin);
}

//------------ serialize for integers and floats --------------//
template <typename Archive, typename T>
typename std::enable_if<std::is_arithmetic<T>::value>::type
serialize(Archive &ar, T &t, const unsigned int ver)
{
    Pothos::serialization::invokeSplit(ar, t, ver);
}

}}
