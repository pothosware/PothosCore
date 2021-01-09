// Copyright (c) 2014-2016 Josh Blum
//                    2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include "SIMD/SIMDConvert.hpp"
#include <Pothos/Config.hpp>
#include <Pothos/Exception.hpp>
#include <Pothos/Util/Templates.hpp>
#include <Pothos/Util/TypeInfo.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/Callable.hpp>
#include <Poco/Format.h>
#include <limits>
#include <complex>
#include <string>
#include <vector>
#include <type_traits>

/***********************************************************************
 * typedefs that make nice clean registry names since we use #name
 **********************************************************************/
typedef unsigned char uchar;
typedef signed char schar;
typedef unsigned short ushort;
typedef signed short sshort;
typedef unsigned int uint;
typedef signed int sint;
typedef unsigned long ulong;
typedef signed long slong;
typedef unsigned long long ullong;
typedef signed long long sllong;
typedef std::complex<signed char> cschar;
typedef std::complex<signed short> csshort;
typedef std::complex<signed int> csint;
typedef std::complex<signed long> cslong;
typedef std::complex<signed long long> csllong;
typedef std::complex<float> cfloat;
typedef std::complex<double> cdouble;

/***********************************************************************
 * type conversion checkers
 **********************************************************************/

//! check if the input value can fit into the output type
template <typename OutType, typename InType>
void convertNumCheck(const InType &in)
{
    if (double(in) > double(std::numeric_limits<OutType>::max()) or //too large
        double(in) < double(std::numeric_limits<OutType>::lowest())) //too small
    {
        throw Pothos::RangeException(Poco::format("value %s out of range for output type %s",
            std::to_string(in), Pothos::Util::typeInfoToString(typeid(OutType))));
    }
}

//! we will assign real to an integer, imaginary should be zero
template <typename OutType, typename InType>
void convertNumCheck(const std::complex<InType> &in)
{
    if (in.imag() != 0)
    {
        throw Pothos::RangeException(Poco::format("cannot convert value (%s + I*%s) with imaginary component into real type %s",
            std::to_string(in.real()), std::to_string(in.imag()), Pothos::Util::typeInfoToString(typeid(OutType))));
    }
    convertNumCheck<OutType>(in.real());
}

/***********************************************************************
 * template comprehension to handle complex and regular numbers
 **********************************************************************/
template <typename InType, typename OutType>
void convertNumHelper(const InType &in, OutType &out)
{
    convertNumCheck<OutType>(in);
    out = OutType(in);
}

template <typename InType, typename OutType>
void convertNumHelper(const std::complex<InType> &in, OutType &out)
{
    convertNumCheck<OutType>(in);
    out = OutType(in.real());
}

template <typename InType, typename OutType>
void convertNumHelper(const InType &in, std::complex<OutType> &out)
{
    out = std::complex<OutType>(OutType(in));
}

template <typename InType, typename OutType>
void convertNumHelper(const std::complex<InType> &in, std::complex<OutType> &out)
{
    out = std::complex<OutType>(OutType(in.real()), OutType(in.imag()));
}

template <typename InType, typename OutType>
OutType convertNum(const InType &in)
{
    OutType out;
    convertNumHelper(in, out);
    return out;
}

/***********************************************************************
 * helper function registers a converter for specific types
 **********************************************************************/
static void registerConvertNum(const std::string &inName, const std::string &outName, const Pothos::Callable &callable)
{
    const std::string name = inName + "_to_" + outName;
    auto path = Pothos::PluginPath("/object/convert/numbers").join(name);
    Pothos::PluginRegistry::add(path, callable);
}

/***********************************************************************
 * template comprehension to handle vectors of numbers
 **********************************************************************/
namespace detail
{
    template <typename InType, typename OutType>
    struct NeitherComplex: std::integral_constant<bool,
        !Pothos::Util::is_complex<InType>::value &&
        !Pothos::Util::is_complex<OutType>::value
    > {};

    template <typename InType, typename OutType>
    struct BothComplex: std::integral_constant<bool,
        Pothos::Util::is_complex<InType>::value &&
        Pothos::Util::is_complex<OutType>::value
    > {};

    template <typename InType, typename OutType>
    std::vector<OutType> simdConvertVec(const std::vector<InType>& in)
    {
        std::vector<OutType> out(in.size());
        simdConvertBuffer<InType, OutType>(in.data(), out.data(), in.size());

        return out;
    }

    template <typename InType, typename OutType>
    std::vector<OutType> manualConvertVec(const std::vector<InType>& in)
    {
        std::vector<OutType> out(in.size());
        for (size_t i = 0; i < out.size(); i++)
        {
            out[i] = convertNum<InType, OutType>(in[i]);
        }
        return out;
    }

    /*
     * Policy: for vector conversions, check each input element to make sure it
     * fits in the output type, and throw an exception otherwise. For the cases
     * where this condition is guaranteed, skip this altogether and go with the
     * optimized SIMD implementation.
     */
    template <typename InType, typename OutType>
    struct CanConvertWithoutValidation: std::integral_constant<bool,
        !Pothos::Util::is_complex<InType>::value &&
        !Pothos::Util::is_complex<OutType>::value &&
        !std::is_same<InType, OutType>::value &&
        (sizeof(InType) < sizeof(OutType)) &&
        ((std::is_unsigned<InType>::value && std::is_signed<OutType>::value) || (std::is_signed<InType>::value == std::is_signed<OutType>::value)) &&
        !(std::is_floating_point<InType>::value && std::is_integral<OutType>::value)>
    {};

    template <typename InType, typename OutType>
    inline typename std::enable_if<std::is_same<InType, OutType>::value, std::vector<OutType>>::type
    convertVec(const std::vector<InType> &in)
    {
        return in;
    }

    template <typename InType, typename OutType>
    inline typename std::enable_if<
        CanConvertWithoutValidation<InType, OutType>::value,
        std::vector<OutType>
    >::type
    convertVec(const std::vector<InType> &in)
    {
        return simdConvertVec<InType, OutType>(in);
    }

    /*
     * Cases:
     *  * One type is scalar, one type is complex
     *  * The input type is larger than the output type
     */
    template <typename InType, typename OutType>
    inline typename std::enable_if<
        !CanConvertWithoutValidation<InType, OutType>::value && !BothComplex<InType, OutType>::value && !std::is_same<InType, OutType>::value,
        std::vector<OutType>
    >::type
    convertVec(const std::vector<InType> &in)
    {
        return manualConvertVec<InType, OutType>(in);
    }

    // Let the scalar implementation dictate what to do
    template <typename InType, typename OutType>
    inline typename std::enable_if<BothComplex<InType, OutType>::value && !std::is_same<InType, OutType>::value, std::vector<OutType>>::type
    convertVec(const std::vector<InType> &in)
    {
        return simdConvertVec<InType, OutType>(in);
    }
}

template <typename InType, typename OutType>
inline std::vector<OutType> convertVec(const std::vector<InType> &in)
{
    return detail::convertVec<InType, OutType>(in);
}

/***********************************************************************
 * helper function registers a converter for specific types
 **********************************************************************/
static void registerConvertVec(const std::string &inName, const std::string &outName, const Pothos::Callable &callable)
{
    const std::string name = inName + "_to_" + outName;
    auto path = Pothos::PluginPath("/object/convert/vectors").join(name);
    Pothos::PluginRegistry::add(path, callable);
}

/***********************************************************************
 * macros to declare all conversion combinations
 **********************************************************************/
#define declare_number_conversion2(inType, outType) \
    registerConvertNum(#inType, #outType, Pothos::Callable(&convertNum<inType, outType>)); \
    registerConvertVec(#inType, #outType, Pothos::Callable(&convertVec<inType, outType>));

#define declare_number_conversion1(inType) \
    declare_number_conversion2(inType, char) \
    declare_number_conversion2(inType, uchar) \
    declare_number_conversion2(inType, schar) \
    declare_number_conversion2(inType, ushort) \
    declare_number_conversion2(inType, sshort) \
    declare_number_conversion2(inType, uint) \
    declare_number_conversion2(inType, sint) \
    declare_number_conversion2(inType, ulong) \
    declare_number_conversion2(inType, slong) \
    declare_number_conversion2(inType, ullong) \
    declare_number_conversion2(inType, sllong) \
    declare_number_conversion2(inType, float) \
    declare_number_conversion2(inType, double) \
    declare_number_conversion2(inType, cfloat) \
    declare_number_conversion2(inType, cdouble)
