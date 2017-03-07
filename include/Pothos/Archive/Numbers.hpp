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
/// Copyright (c) 2016-2017 Josh Blum
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

//------------ 32-64 bit signed integer types --------------//
// use signed LEB128 encoding for variable length encoding
// https://en.wikipedia.org/wiki/LEB128
template<typename Archive, typename T>
typename std::enable_if<
    std::is_same<T, signed int>::value or
    std::is_same<T, signed long>::value or
    std::is_same<T, signed long long>::value
>::type save(Archive &ar, const T &t, const unsigned int)
{
    T value(t);
    unsigned char byte;
    do
    {
        byte = static_cast<unsigned char>(value) & 0x7f;
        value >>= 7; //must be arithmetic shift
        const auto signbit = byte & 0x40;
        if ((value != 0 or signbit != 0) and
            (value != -1 or signbit == 0)) byte |= 0x80;
        ar << byte;
    } while ((byte & 0x80) != 0);
}

template<typename Archive, typename T>
typename std::enable_if<
    std::is_same<T, signed int>::value or
    std::is_same<T, signed long>::value or
    std::is_same<T, signed long long>::value
>::type load(Archive &ar, T &t, const unsigned int)
{
    t = T(0);
    unsigned shift(0);
    unsigned char byte;
    do
    {
        ar >> byte;
        t |= T(byte & 0x7f) << shift;
        shift += 7;
    } while ((byte & 0x80) != 0);

    //sign extend the remaining bits when negative
    const auto signbit = byte & 0x40;
    if (signbit != 0 and shift < (sizeof(T)*8)) t |= -(1 << shift);
}

//------------ 32-64 bit unsigned integer types --------------//
// use unsigned LEB128 encoding for variable length encoding
// https://en.wikipedia.org/wiki/LEB128
template<typename Archive, typename T>
typename std::enable_if<
    std::is_same<T, unsigned int>::value or
    std::is_same<T, unsigned long>::value or
    std::is_same<T, unsigned long long>::value
>::type save(Archive &ar, const T &t, const unsigned int)
{
    T value(t);
    unsigned char byte;
    do
    {
        byte = static_cast<unsigned char>(value) & 0x7f;
        value >>= 7;
        if (value != 0) byte |= 0x80;
        ar << byte;
    } while ((byte & 0x80) != 0);
}

template<typename Archive, typename T>
typename std::enable_if<
    std::is_same<T, unsigned int>::value or
    std::is_same<T, unsigned long>::value or
    std::is_same<T, unsigned long long>::value
>::type load(Archive &ar, T &t, const unsigned int)
{
    t = T(0);
    unsigned shift(0);
    unsigned char byte;
    do
    {
        ar >> byte;
        t |= T(byte & 0x7f) << shift;
        shift += 7;
    } while ((byte & 0x80) != 0);
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
