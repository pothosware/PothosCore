///
/// \file Util/QFormat.hpp
///
/// Templated fixed point utilities and Q-format conversions.
///
/// \copyright
/// Copyright (c) 2015-2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#include <Pothos/Config.hpp>
#include <type_traits>
#include <complex>
#include <cmath>

namespace Pothos {
namespace Util {

//! Determine the fundamental data type of T for primitive types.
template <typename T>
struct Fundamental
{
    typedef T Type;
};

//! Overload for getting the fundamental type of std::complex<T>
template <typename T>
struct Fundamental<std::complex<T>>
{
    typedef T Type;
};

/*!
 * Convert from a Q format number.
 *
 * - Floating point inputs are passed-through without change.
 * - Fixed point inputs are down-shifted by the number of fractional bits.
 *
 * \tparam T the output data type
 * \tparam U the input data type
 *
 * \param in the input number in Q format
 * \param n the number of fractional bits
 * \return the output number
 */
template <typename T, typename U>
T fromQ(const U &in, const int n);

/*!
 * Convert from a Q format number.
 * The fractional bits default to half of the fixed point integer size.
 */
template <typename T, typename U>
T fromQ(const U &in);

/*!
 * Convert a floating point number into Q format.
 *
 * - Floating point outputs are passed-through without change.
 * - Fixed point outputs are shifted up by the number of fractional bits.
 *
 * \tparam T the output data type
 * \tparam U the input data type
 *
 * \param in the input number
 * \param n the number of fractional bits
 * \return the output number in Q format
 */
template <typename T, typename U>
T floatToQ(const U &in, const int n);

/*!
 * Convert a floating point number into Q format.
 * The fractional bits default to half of the fixed point integer size.
 */
template <typename T, typename U>
T floatToQ(const U &in);

namespace Detail {

template <typename T, typename U>
T fromQImpl(const U &in, const int, std::false_type)
{
    return T(in);
}

template <typename T, typename U>
T fromQImpl(const U &in, const int n, std::true_type)
{
    return T(in >> n);
}

template <typename T, typename U>
T fromQImpl(const std::complex<U> &in, const int, std::false_type)
{
    return T(in);
}

template <typename T, typename U>
T fromQImpl(const std::complex<U> &in, const int n, std::true_type)
{
    auto real = fromQ<typename T::value_type, U>(in.real(), n);
    auto imag = fromQ<typename T::value_type, U>(in.imag(), n);
    return T(real, imag);
}

template <typename T, typename U>
T floatToQImpl(const U &in, const int, std::false_type)
{
    return T(in);
}

template <typename T, typename U>
T floatToQImpl(const U &in, const int n, std::true_type)
{
    return T(std::ldexp(in, n));
}

template <typename T, typename U>
T floatToQImpl(const std::complex<U> &in, const int, std::false_type)
{
    return T(in);
}

template <typename T, typename U>
T floatToQImpl(const std::complex<U> &in, const int n, std::true_type)
{
    auto real = floatToQ<typename T::value_type, U>(in.real(), n);
    auto imag = floatToQ<typename T::value_type, U>(in.imag(), n);
    return T(real, imag);
}

static_assert(std::is_same<typename Fundamental<double>::Type, double>::value, "Fundamental of number");

static_assert(std::is_same<typename Fundamental<std::complex<double>>::Type, double>::value, "Fundamental of complex");

} //namespace Detail
} //namespace Util
} //namespace Pothos

template <typename T, typename U>
T Pothos::Util::fromQ(const U &in, const int n)
{
    return Pothos::Util::Detail::fromQImpl<T>(in, n, std::is_integral<typename Fundamental<U>::Type>());
}

template <typename T, typename U>
T Pothos::Util::fromQ(const U &in)
{
    const int n = sizeof(typename Fundamental<U>::Type)*4;
    return Pothos::Util::Detail::fromQImpl<T>(in, n, std::is_integral<typename Fundamental<U>::Type>());
}

template <typename T, typename U>
T Pothos::Util::floatToQ(const U &in, const int n)
{
    return Pothos::Util::Detail::floatToQImpl<T>(in, n, std::is_integral<typename Fundamental<T>::Type>());
}

template <typename T, typename U>
T Pothos::Util::floatToQ(const U &in)
{
    const int n = sizeof(typename Fundamental<T>::Type)*4;
    return Pothos::Util::Detail::floatToQImpl<T>(in, n, std::is_integral<typename Fundamental<T>::Type>());
}
