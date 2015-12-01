// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Config.hpp>
#include <type_traits>
#include <complex>
#include <cmath>

template <typename T, typename U>
typename std::enable_if<std::is_floating_point<U>::value, T>::type
fromQ(const U &in)
{
    return T(in);
}

template <typename T, typename U>
typename std::enable_if<std::is_integral<U>::value, T>::type
fromQ(const U &in)
{
    return T(in >> (sizeof(U)*4));
}

template <typename T, typename U>
typename std::enable_if<std::is_floating_point<U>::value, T>::type
fromQ(const std::complex<U> &in)
{
    return T(in);
}

template <typename T, typename U>
typename std::enable_if<std::is_integral<U>::value, T>::type
fromQ(const std::complex<U> &in)
{
    auto real = fromQ<typename T::value_type>(in.real());
    auto imag = fromQ<typename T::value_type>(in.imag());
    return T(real, imag);
}

template <typename T, typename U>
typename std::enable_if<std::is_floating_point<T>::value, T>::type
floatToQ(const U &in)
{
    return T(in);
}

template <typename T, typename U>
typename std::enable_if<std::is_integral<T>::value, T>::type
floatToQ(const U &in)
{
    return T(std::ldexp(in, sizeof(T)*4));
}

template <typename T, typename U>
typename std::enable_if<std::is_floating_point<typename T::value_type>::value, T>::type
floatToQ(const std::complex<U> &in)
{
    return T(in);
}

template <typename T, typename U>
typename std::enable_if<std::is_integral<typename T::value_type>::value, T>::type
floatToQ(const std::complex<U> &in)
{
    auto real = floatToQ<typename T::value_type>(in.real());
    auto imag = floatToQ<typename T::value_type>(in.imag());
    return T(real, imag);
}
