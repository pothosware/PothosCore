// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <complex>

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
typedef std::complex<float> cfloat;
typedef std::complex<double> cdouble;

/***********************************************************************
 * template comprehension to handle complex and regular numbers
 **********************************************************************/
template <typename InType, typename OutType>
void convertNumHelper(const InType &in, OutType &out)
{
    out = OutType(in);
}

template <typename InType, typename OutType>
void convertNumHelper(const std::complex<InType> &in, OutType &out)
{
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
