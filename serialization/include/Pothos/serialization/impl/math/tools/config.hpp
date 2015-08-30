//  Copyright (c) 2006-7 John Maddock
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef POTHOS_MATH_TOOLS_CONFIG_HPP
#define POTHOS_MATH_TOOLS_CONFIG_HPP

#ifdef _MSC_VER
#pragma once
#endif

#include <Pothos/serialization/impl/config.hpp>
#include <Pothos/serialization/impl/cstdint.hpp> // for boost::uintmax_t
#include <Pothos/serialization/impl/detail/workaround.hpp>
#include <algorithm>  // for min and max
#include <Pothos/serialization/impl/config/no_tr1/cmath.hpp>
#include <climits>
#include <cfloat>
#if (defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__))
#  include <math.h>
#endif

#include <Pothos/serialization/impl/math/tools/user.hpp>

#if (defined(__CYGWIN__) || defined(__FreeBSD__) || defined(__NetBSD__) \
   || (defined(__hppa) && !defined(__OpenBSD__)) || (defined(__NO_LONG_DOUBLE_MATH) && (DBL_MANT_DIG != LDBL_MANT_DIG))) \
   && !defined(POTHOS_MATH_NO_LONG_DOUBLE_MATH_FUNCTIONS)
#  define POTHOS_MATH_NO_LONG_DOUBLE_MATH_FUNCTIONS
#endif
#if POTHOS_WORKAROUND(__BORLANDC__, POTHOS_TESTED_AT(0x582))
//
// Borland post 5.8.2 uses Dinkumware's std C lib which
// doesn't have true long double precision.  Earlier
// versions are problematic too:
//
#  define POTHOS_MATH_NO_REAL_CONCEPT_TESTS
#  define POTHOS_MATH_NO_LONG_DOUBLE_MATH_FUNCTIONS
#  define POTHOS_MATH_CONTROL_FP _control87(MCW_EM,MCW_EM)
#  include <float.h>
#endif
#ifdef __IBMCPP__
//
// For reasons I don't unserstand, the tests with IMB's compiler all
// pass at long double precision, but fail with real_concept, those tests
// are disabled for now.  (JM 2012).
#  define POTHOS_MATH_NO_REAL_CONCEPT_TESTS
#endif
#if (defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)) && ((LDBL_MANT_DIG == 106) || (__LDBL_MANT_DIG__ == 106)) && !defined(POTHOS_MATH_NO_LONG_DOUBLE_MATH_FUNCTIONS)
//
// Darwin's rather strange "double double" is rather hard to
// support, it should be possible given enough effort though...
//
#  define POTHOS_MATH_NO_LONG_DOUBLE_MATH_FUNCTIONS
#endif
#if defined(unix) && defined(__INTEL_COMPILER) && (__INTEL_COMPILER <= 1000) && !defined(POTHOS_MATH_NO_LONG_DOUBLE_MATH_FUNCTIONS)
//
// Intel compiler prior to version 10 has sporadic problems
// calling the long double overloads of the std lib math functions:
// calling ::powl is OK, but std::pow(long double, long double) 
// may segfault depending upon the value of the arguments passed 
// and the specific Linux distribution.
//
// We'll be conservative and disable long double support for this compiler.
//
// Comment out this #define and try building the tests to determine whether
// your Intel compiler version has this issue or not.
//
#  define POTHOS_MATH_NO_LONG_DOUBLE_MATH_FUNCTIONS
#endif
#if defined(unix) && defined(__INTEL_COMPILER)
//
// Intel compiler has sporadic issues compiling std::fpclassify depending on
// the exact OS version used.  Use our own code for this as we know it works
// well on Intel processors:
//
#define POTHOS_MATH_DISABLE_STD_FPCLASSIFY
#endif

#if defined(POTHOS_MSVC) && !defined(_WIN32_WCE)
   // Better safe than sorry, our tests don't support hardware exceptions:
#  define POTHOS_MATH_CONTROL_FP _control87(MCW_EM,MCW_EM)
#endif

#ifdef __IBMCPP__
#  define POTHOS_MATH_NO_DEDUCED_FUNCTION_POINTERS
#endif

#if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901))
#  define POTHOS_MATH_USE_C99
#endif

#if (defined(__hpux) && !defined(__hppa))
#  define POTHOS_MATH_USE_C99
#endif

#if defined(__GNUC__) && defined(_GLIBCXX_USE_C99)
#  define POTHOS_MATH_USE_C99
#endif

#if defined(_LIBCPP_VERSION) && !defined(_MSC_VER)
#  define POTHOS_MATH_USE_C99
#endif

#if defined(__CYGWIN__) || defined(__HP_aCC) || defined(POTHOS_INTEL) \
  || defined(POTHOS_NO_NATIVE_LONG_DOUBLE_FP_CLASSIFY) \
  || (defined(__GNUC__) && !defined(POTHOS_MATH_USE_C99))\
  || defined(POTHOS_MATH_NO_LONG_DOUBLE_MATH_FUNCTIONS)
#  define POTHOS_MATH_NO_NATIVE_LONG_DOUBLE_FP_CLASSIFY
#endif

#if defined(POTHOS_NO_EXPLICIT_FUNCTION_TEMPLATE_ARGUMENTS) || POTHOS_WORKAROUND(__SUNPRO_CC, <= 0x590)

#  include "Pothos/serialization/impl/type.hpp"
#  include "Pothos/serialization/impl/non_type.hpp"

#  define POTHOS_MATH_EXPLICIT_TEMPLATE_TYPE(t)         Pothos::type<t>* = 0
#  define POTHOS_MATH_EXPLICIT_TEMPLATE_TYPE_SPEC(t)    Pothos::type<t>*
#  define POTHOS_MATH_EXPLICIT_TEMPLATE_NON_TYPE(t, v)  Pothos::non_type<t, v>* = 0
#  define POTHOS_MATH_EXPLICIT_TEMPLATE_NON_TYPE_SPEC(t, v)  Pothos::non_type<t, v>*

#  define POTHOS_MATH_APPEND_EXPLICIT_TEMPLATE_TYPE(t)         \
             , POTHOS_MATH_EXPLICIT_TEMPLATE_TYPE(t)
#  define POTHOS_MATH_APPEND_EXPLICIT_TEMPLATE_TYPE_SPEC(t)    \
             , POTHOS_MATH_EXPLICIT_TEMPLATE_TYPE_SPEC(t)
#  define POTHOS_MATH_APPEND_EXPLICIT_TEMPLATE_NON_TYPE(t, v)  \
             , POTHOS_MATH_EXPLICIT_TEMPLATE_NON_TYPE(t, v)
#  define POTHOS_MATH_APPEND_EXPLICIT_TEMPLATE_NON_TYPE_SPEC(t, v)  \
             , POTHOS_MATH_EXPLICIT_TEMPLATE_NON_TYPE_SPEC(t, v)

#else

// no workaround needed: expand to nothing

#  define POTHOS_MATH_EXPLICIT_TEMPLATE_TYPE(t)
#  define POTHOS_MATH_EXPLICIT_TEMPLATE_TYPE_SPEC(t)
#  define POTHOS_MATH_EXPLICIT_TEMPLATE_NON_TYPE(t, v)
#  define POTHOS_MATH_EXPLICIT_TEMPLATE_NON_TYPE_SPEC(t, v)

#  define POTHOS_MATH_APPEND_EXPLICIT_TEMPLATE_TYPE(t)
#  define POTHOS_MATH_APPEND_EXPLICIT_TEMPLATE_TYPE_SPEC(t)
#  define POTHOS_MATH_APPEND_EXPLICIT_TEMPLATE_NON_TYPE(t, v)
#  define POTHOS_MATH_APPEND_EXPLICIT_TEMPLATE_NON_TYPE_SPEC(t, v)


#endif // defined BOOST_NO_EXPLICIT_FUNCTION_TEMPLATE_ARGUMENTS

#if (defined(__SUNPRO_CC) || defined(__hppa) || defined(__GNUC__)) && !defined(POTHOS_MATH_SMALL_CONSTANT)
// Sun's compiler emits a hard error if a constant underflows,
// as does aCC on PA-RISC, while gcc issues a large number of warnings:
#  define POTHOS_MATH_SMALL_CONSTANT(x) 0
#else
#  define POTHOS_MATH_SMALL_CONSTANT(x) x
#endif


#if POTHOS_WORKAROUND(POTHOS_MSVC, < 1400)
//
// Define if constants too large for a float cause "bad"
// values to be stored in the data, rather than infinity
// or a suitably large value.
//
#  define POTHOS_MATH_BUGGY_LARGE_FLOAT_CONSTANTS
#endif
//
// Tune performance options for specific compilers:
//
#ifdef POTHOS_MSVC
#  define POTHOS_MATH_POLY_METHOD 2
#elif defined(POTHOS_INTEL)
#  define POTHOS_MATH_POLY_METHOD 2
#  define POTHOS_MATH_RATIONAL_METHOD 2
#elif defined(__GNUC__)
#  define POTHOS_MATH_POLY_METHOD 3
#  define POTHOS_MATH_RATIONAL_METHOD 3
#  define POTHOS_MATH_INT_TABLE_TYPE(RT, IT) RT
#  define POTHOS_MATH_INT_VALUE_SUFFIX(RV, SUF) RV##.0L
#endif

#if defined(POTHOS_NO_LONG_LONG) && !defined(POTHOS_MATH_INT_TABLE_TYPE)
#  define POTHOS_MATH_INT_TABLE_TYPE(RT, IT) RT
#  define POTHOS_MATH_INT_VALUE_SUFFIX(RV, SUF) RV##.0L
#endif

//
// The maximum order of polynomial that will be evaluated 
// via an unrolled specialisation:
//
#ifndef POTHOS_MATH_MAX_POLY_ORDER
#  define POTHOS_MATH_MAX_POLY_ORDER 17
#endif 
//
// Set the method used to evaluate polynomials and rationals:
//
#ifndef POTHOS_MATH_POLY_METHOD
#  define POTHOS_MATH_POLY_METHOD 1
#endif 
#ifndef POTHOS_MATH_RATIONAL_METHOD
#  define POTHOS_MATH_RATIONAL_METHOD 0
#endif 
//
// decide whether to store constants as integers or reals:
//
#ifndef POTHOS_MATH_INT_TABLE_TYPE
#  define POTHOS_MATH_INT_TABLE_TYPE(RT, IT) IT
#endif
#ifndef POTHOS_MATH_INT_VALUE_SUFFIX
#  define POTHOS_MATH_INT_VALUE_SUFFIX(RV, SUF) RV##SUF
#endif
//
// Test whether to support __float128:
//
#if defined(_GLIBCXX_USE_FLOAT128) && defined(POTHOS_GCC) && !defined(__STRICT_ANSI__)
//
// Only enable this when the compiler really is GCC as clang and probably 
// intel too don't support __float128 yet :-(
//
#  define POTHOS_MATH_USE_FLOAT128
#endif
//
// Check for WinCE with no iostream support:
//
#if defined(_WIN32_WCE) && !defined(__SGI_STL_PORT)
#  define POTHOS_MATH_NO_LEXICAL_CAST
#endif

//
// Helper macro for controlling the FP behaviour:
//
#ifndef POTHOS_MATH_CONTROL_FP
#  define POTHOS_MATH_CONTROL_FP
#endif
//
// Helper macro for using statements:
//
#define POTHOS_MATH_STD_USING_CORE \
   using std::abs;\
   using std::acos;\
   using std::cos;\
   using std::fmod;\
   using std::modf;\
   using std::tan;\
   using std::asin;\
   using std::cosh;\
   using std::frexp;\
   using std::pow;\
   using std::tanh;\
   using std::atan;\
   using std::exp;\
   using std::ldexp;\
   using std::sin;\
   using std::atan2;\
   using std::fabs;\
   using std::log;\
   using std::sinh;\
   using std::ceil;\
   using std::floor;\
   using std::log10;\
   using std::sqrt;

#define POTHOS_MATH_STD_USING POTHOS_MATH_STD_USING_CORE

namespace Pothos{ namespace math{
namespace tools
{

template <class T>
inline T max POTHOS_PREVENT_MACRO_SUBSTITUTION(T a, T b, T c)
{
   return (std::max)((std::max)(a, b), c);
}

template <class T>
inline T max POTHOS_PREVENT_MACRO_SUBSTITUTION(T a, T b, T c, T d)
{
   return (std::max)((std::max)(a, b), (std::max)(c, d));
}

} // namespace tools

template <class T>
void suppress_unused_variable_warning(const T&)
{
}

}} // namespace boost namespace math

#if ((defined(__linux__) && !defined(__UCLIBC__)) || defined(__QNX__) || defined(__IBMCPP__)) && !defined(POTHOS_NO_FENV_H)

   #include <Pothos/serialization/impl/detail/fenv.hpp>

#  ifdef FE_ALL_EXCEPT

namespace Pothos{ namespace math{
   namespace detail
   {
   struct fpu_guard
   {
      fpu_guard()
      {
         fegetexceptflag(&m_flags, FE_ALL_EXCEPT);
         feclearexcept(FE_ALL_EXCEPT);
      }
      ~fpu_guard()
      {
         fesetexceptflag(&m_flags, FE_ALL_EXCEPT);
      }
   private:
      fexcept_t m_flags;
   };

   } // namespace detail
   }} // namespaces

#    define POTHOS_FPU_EXCEPTION_GUARD Pothos::math::detail::fpu_guard local_guard_object;
#    define POTHOS_MATH_INSTRUMENT_FPU do{ fexcept_t cpu_flags; fegetexceptflag(&cpu_flags, FE_ALL_EXCEPT); POTHOS_MATH_INSTRUMENT_VARIABLE(cpu_flags); } while(0); 

#  else

#    define POTHOS_FPU_EXCEPTION_GUARD
#    define POTHOS_MATH_INSTRUMENT_FPU

#  endif

#else // All other platforms.
#  define POTHOS_FPU_EXCEPTION_GUARD
#  define POTHOS_MATH_INSTRUMENT_FPU
#endif

#ifdef POTHOS_MATH_INSTRUMENT

#  include <iostream>
#  include <iomanip>
#  include <typeinfo>

#  define POTHOS_MATH_INSTRUMENT_CODE(x) \
      std::cout << std::setprecision(35) << __FILE__ << ":" << __LINE__ << " " << x << std::endl;
#  define POTHOS_MATH_INSTRUMENT_VARIABLE(name) POTHOS_MATH_INSTRUMENT_CODE(POTHOS_STRINGIZE(name) << " = " << name)

#else

#  define POTHOS_MATH_INSTRUMENT_CODE(x)
#  define POTHOS_MATH_INSTRUMENT_VARIABLE(name)

#endif

#endif // BOOST_MATH_TOOLS_CONFIG_HPP





