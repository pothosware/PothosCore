#pragma once
#include <cstdint>
// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
namespace spuce {
//! \file
//! \brief Determine resultant type when mixing two types in math operations
//! \author Tony Kirke,  Copyright(c) 2005
//! \ingroup traits traits
//
// This template is used to look at two types used together in some mathematical expression
// and then figure out the correct type for the result of the math expression
// For example (addition)
// input types                       Result type
// double + double                   = double
// double + int64_t                  = double
// double + complex<double>          = complex<double>
// double + complex<int64_t>         = complex<double>
// int64_t   + int64_t               = int64_t
// int64_t   + complex<double>       = complex<double>
// int64_t   + complex<int64_t>      = complex<int64_t>
// complex<double> + complex<double>       = complex<double>
// complex<double> + complex<int64_t>      = complex<double>
// complex<int64_t>   + complex<int64_t>   = complex<int64_t>
template <typename T, typename D = double> class mixed_type {
 public:
  typedef double dtype;
};
#define MIXED_MACRO(T1, T2, RESULT)      \
  template <> class mixed_type<T1, T2> { \
   public:                               \
    typedef RESULT dtype;                \
  };
#define DMIXED_MACRO(T1, T2, RESULT)     \
  template <> class mixed_type<T1, T2> { \
   public:                               \
    typedef RESULT dtype;                \
  };                                     \
  template <> class mixed_type<T2, T1> { \
   public:                               \
    typedef RESULT dtype;                \
  };
//----------------------------------------------------------
MIXED_MACRO(int64_t, int64_t, int64_t)
DMIXED_MACRO(int64_t, int32_t, int64_t)
DMIXED_MACRO(int64_t, int16_t, int64_t)
DMIXED_MACRO(int64_t, int8_t, int64_t)
DMIXED_MACRO(int64_t, double, double)
DMIXED_MACRO(int64_t, float, float)

MIXED_MACRO(int32_t, int32_t, int32_t)
DMIXED_MACRO(int32_t, int16_t, double)
DMIXED_MACRO(int32_t, int8_t, double)
DMIXED_MACRO(int32_t, double, double)
DMIXED_MACRO(int32_t, float, float)

MIXED_MACRO(int16_t, int16_t, int16_t)
DMIXED_MACRO(int16_t, int8_t, double)
DMIXED_MACRO(int16_t, double, double)
DMIXED_MACRO(int16_t, float, float)

MIXED_MACRO(int8_t, int8_t, int8_t)
DMIXED_MACRO(int8_t, double, double)
DMIXED_MACRO(int8_t, float, float)

MIXED_MACRO(double, double, double)
DMIXED_MACRO(double, float, double)
MIXED_MACRO(float, float, float)

MIXED_MACRO(std::complex<int64_t>, std::complex<int64_t>, std::complex<int64_t>)
DMIXED_MACRO(std::complex<int64_t>, std::complex<int32_t>, std::complex<int64_t>)
DMIXED_MACRO(std::complex<int64_t>, std::complex<int16_t>, std::complex<int64_t>)
DMIXED_MACRO(std::complex<int64_t>, std::complex<int8_t>, std::complex<int64_t>)
DMIXED_MACRO(std::complex<int64_t>, std::complex<double>, std::complex<double>)
DMIXED_MACRO(std::complex<int64_t>, std::complex<float>, std::complex<float>)

MIXED_MACRO(std::complex<int32_t>, std::complex<int32_t>, std::complex<int32_t>)
DMIXED_MACRO(std::complex<int32_t>, std::complex<int16_t>, std::complex<int32_t>)
DMIXED_MACRO(std::complex<int32_t>, std::complex<int8_t>, std::complex<int32_t>)
DMIXED_MACRO(std::complex<int32_t>, std::complex<double>, std::complex<double>)
DMIXED_MACRO(std::complex<int32_t>, std::complex<float>, std::complex<float>)

MIXED_MACRO(std::complex<int16_t>, std::complex<int16_t>, std::complex<int16_t>)
MIXED_MACRO(std::complex<int16_t>, std::complex<int8_t>, std::complex<int16_t>)
DMIXED_MACRO(std::complex<int16_t>, std::complex<double>, std::complex<double>)
DMIXED_MACRO(std::complex<int16_t>, std::complex<float>, std::complex<float>)

MIXED_MACRO(std::complex<int8_t>, std::complex<int8_t>, std::complex<int8_t>)
DMIXED_MACRO(std::complex<int8_t>, std::complex<double>, std::complex<double>)
DMIXED_MACRO(std::complex<int8_t>, std::complex<float>, std::complex<float>)

MIXED_MACRO(std::complex<double>, std::complex<double>, std::complex<double>)
DMIXED_MACRO(std::complex<double>, std::complex<float>, std::complex<double>)

MIXED_MACRO(std::complex<float>, std::complex<float>, std::complex<float>)

DMIXED_MACRO(std::complex<int64_t>, int64_t, std::complex<int64_t>)
DMIXED_MACRO(std::complex<int64_t>, int32_t, std::complex<int64_t>)
DMIXED_MACRO(std::complex<int64_t>, int16_t, std::complex<int64_t>)
DMIXED_MACRO(std::complex<int64_t>, int8_t, std::complex<int64_t>)
DMIXED_MACRO(std::complex<int64_t>, double, std::complex<double>)
DMIXED_MACRO(std::complex<int64_t>, float, std::complex<float>)

DMIXED_MACRO(std::complex<int32_t>, int64_t, std::complex<int64_t>)
DMIXED_MACRO(std::complex<int32_t>, int32_t, std::complex<int32_t>)
DMIXED_MACRO(std::complex<int32_t>, int16_t, std::complex<int32_t>)
DMIXED_MACRO(std::complex<int32_t>, int8_t, std::complex<int32_t>)
DMIXED_MACRO(std::complex<int32_t>, double, std::complex<double>)
DMIXED_MACRO(std::complex<int32_t>, float, std::complex<float>)

DMIXED_MACRO(std::complex<int16_t>, int64_t, std::complex<int64_t>)
DMIXED_MACRO(std::complex<int16_t>, int32_t, std::complex<int32_t>)
DMIXED_MACRO(std::complex<int16_t>, int16_t, std::complex<int16_t>)
DMIXED_MACRO(std::complex<int16_t>, int8_t, std::complex<int16_t>)
DMIXED_MACRO(std::complex<int16_t>, double, std::complex<double>)
DMIXED_MACRO(std::complex<int16_t>, float, std::complex<float>)

DMIXED_MACRO(std::complex<int8_t>, int64_t, std::complex<int64_t>)
DMIXED_MACRO(std::complex<int8_t>, int32_t, std::complex<int32_t>)
DMIXED_MACRO(std::complex<int8_t>, int16_t, std::complex<int16_t>)
DMIXED_MACRO(std::complex<int8_t>, int8_t, std::complex<int8_t>)
DMIXED_MACRO(std::complex<int8_t>, double, std::complex<double>)
DMIXED_MACRO(std::complex<int8_t>, float, std::complex<float>)

DMIXED_MACRO(std::complex<double>, int64_t, std::complex<double>)
DMIXED_MACRO(std::complex<double>, int32_t, std::complex<double>)
DMIXED_MACRO(std::complex<double>, int16_t, std::complex<double>)
DMIXED_MACRO(std::complex<double>, int8_t, std::complex<double>)
DMIXED_MACRO(std::complex<double>, double, std::complex<double>)
DMIXED_MACRO(std::complex<double>, float, std::complex<double>)

DMIXED_MACRO(std::complex<float>, int64_t, std::complex<float>)
DMIXED_MACRO(std::complex<float>, int32_t, std::complex<float>)
DMIXED_MACRO(std::complex<float>, int16_t, std::complex<float>)
DMIXED_MACRO(std::complex<float>, int8_t, std::complex<float>)
DMIXED_MACRO(std::complex<float>, double, std::complex<double>)
DMIXED_MACRO(std::complex<float>, float, std::complex<float>)

#undef DMIXED_MACRO
#undef MIXED_MACRO
}  // namespace spuce
