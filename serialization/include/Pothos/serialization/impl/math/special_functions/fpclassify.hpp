//  Copyright John Maddock 2005-2008.
//  Copyright (c) 2006-2008 Johan Rade
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef POTHOS_MATH_FPCLASSIFY_HPP
#define POTHOS_MATH_FPCLASSIFY_HPP

#ifdef _MSC_VER
#pragma once
#endif

#include <math.h>
#include <Pothos/serialization/impl/config/no_tr1/cmath.hpp>
#include <Pothos/serialization/impl/limits.hpp>
#include <Pothos/serialization/impl/math/tools/real_cast.hpp>
#include <Pothos/serialization/impl/type_traits/is_floating_point.hpp>
#include <Pothos/serialization/impl/math/special_functions/math_fwd.hpp>
#include <Pothos/serialization/impl/math/special_functions/detail/fp_traits.hpp>
/*!
  \file fpclassify.hpp
  \brief Classify floating-point value as normal, subnormal, zero, infinite, or NaN.
  \version 1.0
  \author John Maddock
 */

/*

1. If the platform is C99 compliant, then the native floating point
classification functions are used.  However, note that we must only
define the functions which call std::fpclassify etc if that function
really does exist: otherwise a compiler may reject the code even though
the template is never instantiated.

2. If the platform is not C99 compliant, and the binary format for
a floating point type (float, double or long double) can be determined
at compile time, then the following algorithm is used:

        If all exponent bits, the flag bit (if there is one),
        and all significand bits are 0, then the number is zero.

        If all exponent bits and the flag bit (if there is one) are 0,
        and at least one significand bit is 1, then the number is subnormal.

        If all exponent bits are 1 and all significand bits are 0,
        then the number is infinity.

        If all exponent bits are 1 and at least one significand bit is 1,
        then the number is a not-a-number.

        Otherwise the number is normal.

        This algorithm works for the IEEE 754 representation,
        and also for several non IEEE 754 formats.

    Most formats have the structure
        sign bit + exponent bits + significand bits.

    A few have the structure
        sign bit + exponent bits + flag bit + significand bits.
    The flag bit is 0 for zero and subnormal numbers,
        and 1 for normal numbers and NaN.
        It is 0 (Motorola 68K) or 1 (Intel) for infinity.

    To get the bits, the four or eight most significant bytes are copied
    into an uint32_t or uint64_t and bit masks are applied.
    This covers all the exponent bits and the flag bit (if there is one),
    but not always all the significand bits.
    Some of the functions below have two implementations,
    depending on whether all the significand bits are copied or not.

3. If the platform is not C99 compliant, and the binary format for
a floating point type (float, double or long double) can not be determined
at compile time, then comparison with std::numeric_limits values
is used.

*/

#if defined(_MSC_VER) || defined(__BORLANDC__)
#include <float.h>
#endif

#ifdef POTHOS_NO_STDC_NAMESPACE
  namespace std{ using ::abs; using ::fabs; }
#endif

namespace Pothos{

//
// This must not be located in any namespace under boost::math
// otherwise we can get into an infinite loop if isnan is
// a #define for "isnan" !
//
namespace math_detail{

#ifdef POTHOS_MSVC
#pragma warning(push)
#pragma warning(disable:4800)
#endif

template <class T>
inline bool is_nan_helper(T t, const Pothos::true_type&)
{
#ifdef isnan
   return isnan(t);
#elif defined(POTHOS_MATH_DISABLE_STD_FPCLASSIFY) || !defined(POTHOS_HAS_FPCLASSIFY)
   (void)t;
   return false;
#else // BOOST_HAS_FPCLASSIFY
   return (POTHOS_FPCLASSIFY_PREFIX fpclassify(t) == (int)FP_NAN);
#endif
}

#ifdef POTHOS_MSVC
#pragma warning(pop)
#endif

template <class T>
inline bool is_nan_helper(T, const Pothos::false_type&)
{
   return false;
}

}

namespace math{

namespace detail{

#ifdef POTHOS_MATH_USE_STD_FPCLASSIFY
template <class T>
inline int fpclassify_imp POTHOS_NO_MACRO_EXPAND(T t, const native_tag&)
{
   return (std::fpclassify)(t);
}
#endif

template <class T>
inline int fpclassify_imp POTHOS_NO_MACRO_EXPAND(T t, const generic_tag<true>&)
{
   POTHOS_MATH_INSTRUMENT_VARIABLE(t);

   // whenever possible check for Nan's first:
#if defined(POTHOS_HAS_FPCLASSIFY)  && !defined(POTHOS_MATH_DISABLE_STD_FPCLASSIFY)
   if(::Pothos::math_detail::is_nan_helper(t, ::Pothos::is_floating_point<T>()))
      return FP_NAN;
#elif defined(isnan)
   if(Pothos::math_detail::is_nan_helper(t, ::Pothos::is_floating_point<T>()))
      return FP_NAN;
#elif defined(_MSC_VER) || defined(__BORLANDC__)
   if(::_isnan(Pothos::math::tools::real_cast<double>(t)))
      return FP_NAN;
#endif
   // std::fabs broken on a few systems especially for long long!!!!
   T at = (t < T(0)) ? -t : t;

   // Use a process of exclusion to figure out
   // what kind of type we have, this relies on
   // IEEE conforming reals that will treat
   // Nan's as unordered.  Some compilers
   // don't do this once optimisations are
   // turned on, hence the check for nan's above.
   if(at <= (std::numeric_limits<T>::max)())
   {
      if(at >= (std::numeric_limits<T>::min)())
         return FP_NORMAL;
      return (at != 0) ? FP_SUBNORMAL : FP_ZERO;
   }
   else if(at > (std::numeric_limits<T>::max)())
      return FP_INFINITE;
   return FP_NAN;
}

template <class T>
inline int fpclassify_imp POTHOS_NO_MACRO_EXPAND(T t, const generic_tag<false>&)
{
#ifdef POTHOS_NO_LIMITS_COMPILE_TIME_CONSTANTS
   if(std::numeric_limits<T>::is_specialized)
      return fpclassify_imp(t, generic_tag<true>());
#endif
   //
   // An unknown type with no numeric_limits support,
   // so what are we supposed to do we do here?
   //
   POTHOS_MATH_INSTRUMENT_VARIABLE(t);

   return t == 0 ? FP_ZERO : FP_NORMAL;
}

template<class T>
int fpclassify_imp POTHOS_NO_MACRO_EXPAND(T x, ieee_copy_all_bits_tag)
{
   typedef POTHOS_DEDUCED_TYPENAME fp_traits<T>::type traits;

   POTHOS_MATH_INSTRUMENT_VARIABLE(x);

   POTHOS_DEDUCED_TYPENAME traits::bits a;
   traits::get_bits(x,a);
   POTHOS_MATH_INSTRUMENT_VARIABLE(a);
   a &= traits::exponent | traits::flag | traits::significand;
   POTHOS_MATH_INSTRUMENT_VARIABLE((traits::exponent | traits::flag | traits::significand));
   POTHOS_MATH_INSTRUMENT_VARIABLE(a);

   if(a <= traits::significand) {
      if(a == 0)
         return FP_ZERO;
      else
         return FP_SUBNORMAL;
   }

   if(a < traits::exponent) return FP_NORMAL;

   a &= traits::significand;
   if(a == 0) return FP_INFINITE;

   return FP_NAN;
}

template<class T>
int fpclassify_imp POTHOS_NO_MACRO_EXPAND(T x, ieee_copy_leading_bits_tag)
{
   typedef POTHOS_DEDUCED_TYPENAME fp_traits<T>::type traits;

   POTHOS_MATH_INSTRUMENT_VARIABLE(x);

   POTHOS_DEDUCED_TYPENAME traits::bits a;
   traits::get_bits(x,a);
   a &= traits::exponent | traits::flag | traits::significand;

   if(a <= traits::significand) {
      if(x == 0)
         return FP_ZERO;
      else
         return FP_SUBNORMAL;
   }

   if(a < traits::exponent) return FP_NORMAL;

   a &= traits::significand;
   traits::set_bits(x,a);
   if(x == 0) return FP_INFINITE;

   return FP_NAN;
}

#if defined(POTHOS_MATH_USE_STD_FPCLASSIFY) && (defined(POTHOS_MATH_NO_NATIVE_LONG_DOUBLE_FP_CLASSIFY) || defined(POTHOS_MATH_NO_LONG_DOUBLE_MATH_FUNCTIONS))
inline int fpclassify_imp POTHOS_NO_MACRO_EXPAND(long double t, const native_tag&)
{
   return Pothos::math::detail::fpclassify_imp(t, generic_tag<true>());
}
#endif

}  // namespace detail

template <class T>
inline int fpclassify POTHOS_NO_MACRO_EXPAND(T t)
{
   typedef typename detail::fp_traits<T>::type traits;
   typedef typename traits::method method;
   typedef typename tools::promote_args_permissive<T>::type value_type;
#ifdef POTHOS_NO_LIMITS_COMPILE_TIME_CONSTANTS
   if(std::numeric_limits<T>::is_specialized && detail::is_generic_tag_false(static_cast<method*>(0)))
      return detail::fpclassify_imp(static_cast<value_type>(t), detail::generic_tag<true>());
   return detail::fpclassify_imp(static_cast<value_type>(t), method());
#else
   return detail::fpclassify_imp(static_cast<value_type>(t), method());
#endif
}

#ifdef POTHOS_MATH_NO_LONG_DOUBLE_MATH_FUNCTIONS
template <>
inline int fpclassify<long double> POTHOS_NO_MACRO_EXPAND(long double t)
{
   typedef detail::fp_traits<long double>::type traits;
   typedef traits::method method;
   typedef long double value_type;
#ifdef POTHOS_NO_LIMITS_COMPILE_TIME_CONSTANTS
   if(std::numeric_limits<long double>::is_specialized && detail::is_generic_tag_false(static_cast<method*>(0)))
      return detail::fpclassify_imp(static_cast<value_type>(t), detail::generic_tag<true>());
   return detail::fpclassify_imp(static_cast<value_type>(t), method());
#else
   return detail::fpclassify_imp(static_cast<value_type>(t), method());
#endif
}
#endif

namespace detail {

#ifdef POTHOS_MATH_USE_STD_FPCLASSIFY
    template<class T>
    inline bool isfinite_impl(T x, native_tag const&)
    {
        return (std::isfinite)(x);
    }
#endif

    template<class T>
    inline bool isfinite_impl(T x, generic_tag<true> const&)
    {
        return x >= -(std::numeric_limits<T>::max)()
            && x <= (std::numeric_limits<T>::max)();
    }

    template<class T>
    inline bool isfinite_impl(T x, generic_tag<false> const&)
    {
#ifdef POTHOS_NO_LIMITS_COMPILE_TIME_CONSTANTS
      if(std::numeric_limits<T>::is_specialized)
         return isfinite_impl(x, generic_tag<true>());
#endif
       (void)x; // warning supression.
       return true;
    }

    template<class T>
    inline bool isfinite_impl(T x, ieee_tag const&)
    {
        typedef POTHOS_DEDUCED_TYPENAME detail::fp_traits<T>::type traits;
        POTHOS_DEDUCED_TYPENAME traits::bits a;
        traits::get_bits(x,a);
        a &= traits::exponent;
        return a != traits::exponent;
    }

#if defined(POTHOS_MATH_USE_STD_FPCLASSIFY) && defined(POTHOS_MATH_NO_NATIVE_LONG_DOUBLE_FP_CLASSIFY)
inline bool isfinite_impl POTHOS_NO_MACRO_EXPAND(long double t, const native_tag&)
{
   return Pothos::math::detail::isfinite_impl(t, generic_tag<true>());
}
#endif

}

template<class T>
inline bool (isfinite)(T x)
{ //!< \brief return true if floating-point type t is finite.
   typedef typename detail::fp_traits<T>::type traits;
   typedef typename traits::method method;
   // typedef typename boost::is_floating_point<T>::type fp_tag;
   typedef typename tools::promote_args_permissive<T>::type value_type;
   return detail::isfinite_impl(static_cast<value_type>(x), method());
}

#ifdef POTHOS_MATH_NO_LONG_DOUBLE_MATH_FUNCTIONS
template<>
inline bool (isfinite)(long double x)
{ //!< \brief return true if floating-point type t is finite.
   typedef detail::fp_traits<long double>::type traits;
   typedef traits::method method;
   typedef Pothos::is_floating_point<long double>::type fp_tag;
   typedef long double value_type;
   return detail::isfinite_impl(static_cast<value_type>(x), method());
}
#endif

//------------------------------------------------------------------------------

namespace detail {

#ifdef POTHOS_MATH_USE_STD_FPCLASSIFY
    template<class T>
    inline bool isnormal_impl(T x, native_tag const&)
    {
        return (std::isnormal)(x);
    }
#endif

    template<class T>
    inline bool isnormal_impl(T x, generic_tag<true> const&)
    {
        if(x < 0) x = -x;
        return x >= (std::numeric_limits<T>::min)()
            && x <= (std::numeric_limits<T>::max)();
    }

    template<class T>
    inline bool isnormal_impl(T x, generic_tag<false> const&)
    {
#ifdef POTHOS_NO_LIMITS_COMPILE_TIME_CONSTANTS
      if(std::numeric_limits<T>::is_specialized)
         return isnormal_impl(x, generic_tag<true>());
#endif
       return !(x == 0);
    }

    template<class T>
    inline bool isnormal_impl(T x, ieee_tag const&)
    {
        typedef POTHOS_DEDUCED_TYPENAME detail::fp_traits<T>::type traits;
        POTHOS_DEDUCED_TYPENAME traits::bits a;
        traits::get_bits(x,a);
        a &= traits::exponent | traits::flag;
        return (a != 0) && (a < traits::exponent);
    }

#if defined(POTHOS_MATH_USE_STD_FPCLASSIFY) && defined(POTHOS_MATH_NO_NATIVE_LONG_DOUBLE_FP_CLASSIFY)
inline bool isnormal_impl POTHOS_NO_MACRO_EXPAND(long double t, const native_tag&)
{
   return Pothos::math::detail::isnormal_impl(t, generic_tag<true>());
}
#endif

}

template<class T>
inline bool (isnormal)(T x)
{
   typedef typename detail::fp_traits<T>::type traits;
   typedef typename traits::method method;
   //typedef typename boost::is_floating_point<T>::type fp_tag;
   typedef typename tools::promote_args_permissive<T>::type value_type;
   return detail::isnormal_impl(static_cast<value_type>(x), method());
}

#ifdef POTHOS_MATH_NO_LONG_DOUBLE_MATH_FUNCTIONS
template<>
inline bool (isnormal)(long double x)
{
   typedef detail::fp_traits<long double>::type traits;
   typedef traits::method method;
   typedef Pothos::is_floating_point<long double>::type fp_tag;
   typedef long double value_type;
   return detail::isnormal_impl(static_cast<value_type>(x), method());
}
#endif

//------------------------------------------------------------------------------

namespace detail {

#ifdef POTHOS_MATH_USE_STD_FPCLASSIFY
    template<class T>
    inline bool isinf_impl(T x, native_tag const&)
    {
        return (std::isinf)(x);
    }
#endif

    template<class T>
    inline bool isinf_impl(T x, generic_tag<true> const&)
    {
        (void)x; // in case the compiler thinks that x is unused because std::numeric_limits<T>::has_infinity is false
        return std::numeric_limits<T>::has_infinity
            && ( x == std::numeric_limits<T>::infinity()
                 || x == -std::numeric_limits<T>::infinity());
    }

    template<class T>
    inline bool isinf_impl(T x, generic_tag<false> const&)
    {
#ifdef POTHOS_NO_LIMITS_COMPILE_TIME_CONSTANTS
      if(std::numeric_limits<T>::is_specialized)
         return isinf_impl(x, generic_tag<true>());
#endif
        (void)x; // warning supression.
        return false;
    }

    template<class T>
    inline bool isinf_impl(T x, ieee_copy_all_bits_tag const&)
    {
        typedef POTHOS_DEDUCED_TYPENAME fp_traits<T>::type traits;

        POTHOS_DEDUCED_TYPENAME traits::bits a;
        traits::get_bits(x,a);
        a &= traits::exponent | traits::significand;
        return a == traits::exponent;
    }

    template<class T>
    inline bool isinf_impl(T x, ieee_copy_leading_bits_tag const&)
    {
        typedef POTHOS_DEDUCED_TYPENAME fp_traits<T>::type traits;

        POTHOS_DEDUCED_TYPENAME traits::bits a;
        traits::get_bits(x,a);
        a &= traits::exponent | traits::significand;
        if(a != traits::exponent)
            return false;

        traits::set_bits(x,0);
        return x == 0;
    }

#if defined(POTHOS_MATH_USE_STD_FPCLASSIFY) && defined(POTHOS_MATH_NO_NATIVE_LONG_DOUBLE_FP_CLASSIFY)
inline bool isinf_impl POTHOS_NO_MACRO_EXPAND(long double t, const native_tag&)
{
   return Pothos::math::detail::isinf_impl(t, generic_tag<true>());
}
#endif

}   // namespace detail

template<class T>
inline bool (isinf)(T x)
{
   typedef typename detail::fp_traits<T>::type traits;
   typedef typename traits::method method;
   // typedef typename boost::is_floating_point<T>::type fp_tag;
   typedef typename tools::promote_args_permissive<T>::type value_type;
   return detail::isinf_impl(static_cast<value_type>(x), method());
}

#ifdef POTHOS_MATH_NO_LONG_DOUBLE_MATH_FUNCTIONS
template<>
inline bool (isinf)(long double x)
{
   typedef detail::fp_traits<long double>::type traits;
   typedef traits::method method;
   typedef Pothos::is_floating_point<long double>::type fp_tag;
   typedef long double value_type;
   return detail::isinf_impl(static_cast<value_type>(x), method());
}
#endif

//------------------------------------------------------------------------------

namespace detail {

#ifdef POTHOS_MATH_USE_STD_FPCLASSIFY
    template<class T>
    inline bool isnan_impl(T x, native_tag const&)
    {
        return (std::isnan)(x);
    }
#endif

    template<class T>
    inline bool isnan_impl(T x, generic_tag<true> const&)
    {
        return std::numeric_limits<T>::has_infinity
            ? !(x <= std::numeric_limits<T>::infinity())
            : x != x;
    }

    template<class T>
    inline bool isnan_impl(T x, generic_tag<false> const&)
    {
#ifdef POTHOS_NO_LIMITS_COMPILE_TIME_CONSTANTS
      if(std::numeric_limits<T>::is_specialized)
         return isnan_impl(x, generic_tag<true>());
#endif
        (void)x; // warning supression
        return false;
    }

    template<class T>
    inline bool isnan_impl(T x, ieee_copy_all_bits_tag const&)
    {
        typedef POTHOS_DEDUCED_TYPENAME fp_traits<T>::type traits;

        POTHOS_DEDUCED_TYPENAME traits::bits a;
        traits::get_bits(x,a);
        a &= traits::exponent | traits::significand;
        return a > traits::exponent;
    }

    template<class T>
    inline bool isnan_impl(T x, ieee_copy_leading_bits_tag const&)
    {
        typedef POTHOS_DEDUCED_TYPENAME fp_traits<T>::type traits;

        POTHOS_DEDUCED_TYPENAME traits::bits a;
        traits::get_bits(x,a);

        a &= traits::exponent | traits::significand;
        if(a < traits::exponent)
            return false;

        a &= traits::significand;
        traits::set_bits(x,a);
        return x != 0;
    }

}   // namespace detail

template<class T>
inline bool (isnan)(T x)
{ //!< \brief return true if floating-point type t is NaN (Not A Number).
   typedef typename detail::fp_traits<T>::type traits;
   typedef typename traits::method method;
   // typedef typename boost::is_floating_point<T>::type fp_tag;
   return detail::isnan_impl(x, method());
}

#ifdef isnan
template <> inline bool isnan POTHOS_NO_MACRO_EXPAND<float>(float t){ return ::Pothos::math_detail::is_nan_helper(t, Pothos::true_type()); }
template <> inline bool isnan POTHOS_NO_MACRO_EXPAND<double>(double t){ return ::Pothos::math_detail::is_nan_helper(t, Pothos::true_type()); }
template <> inline bool isnan POTHOS_NO_MACRO_EXPAND<long double>(long double t){ return ::Pothos::math_detail::is_nan_helper(t, Pothos::true_type()); }
#elif defined(POTHOS_MATH_NO_LONG_DOUBLE_MATH_FUNCTIONS)
template<>
inline bool (isnan)(long double x)
{ //!< \brief return true if floating-point type t is NaN (Not A Number).
   typedef detail::fp_traits<long double>::type traits;
   typedef traits::method method;
   typedef Pothos::is_floating_point<long double>::type fp_tag;
   return detail::isnan_impl(x, method());
}
#endif

} // namespace math
} // namespace boost

#endif // BOOST_MATH_FPCLASSIFY_HPP

