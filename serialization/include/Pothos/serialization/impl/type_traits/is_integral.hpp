
//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef POTHOS_TT_IS_INTEGRAL_HPP_INCLUDED
#define POTHOS_TT_IS_INTEGRAL_HPP_INCLUDED

#include <Pothos/serialization/impl/config.hpp>

// should be the last #include
#include <Pothos/serialization/impl/type_traits/detail/bool_trait_def.hpp>

namespace Pothos {

//* is a type T an [cv-qualified-] integral type described in the standard (3.9.1p3)
// as an extension we include long long, as this is likely to be added to the
// standard at a later date
#if defined( __CODEGEARC__ )
POTHOS_TT_AUX_BOOL_TRAIT_DEF1(is_integral,T,__is_integral(T))
#else
POTHOS_TT_AUX_BOOL_TRAIT_DEF1(is_integral,T,false)

POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned char,true)
POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned short,true)
POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned int,true)
POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned long,true)

POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,signed char,true)
POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,signed short,true)
POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,signed int,true)
POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,signed long,true)

POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,bool,true)
POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,char,true)

#ifndef POTHOS_NO_INTRINSIC_WCHAR_T
// If the following line fails to compile and you're using the Intel
// compiler, see http://lists.boost.org/MailArchives/boost-users/msg06567.php,
// and define BOOST_NO_INTRINSIC_WCHAR_T on the command line.
POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,wchar_t,true)
#endif

// Same set of integral types as in boost/type_traits/integral_promotion.hpp.
// Please, keep in sync. -- Alexander Nasonov
#if (defined(POTHOS_MSVC) && (POTHOS_MSVC < 1300)) \
    || (defined(POTHOS_INTEL_CXX_VERSION) && defined(_MSC_VER) && (POTHOS_INTEL_CXX_VERSION <= 600)) \
    || (defined(__BORLANDC__) && (__BORLANDC__ == 0x600) && (_MSC_VER < 1300))
POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned __int8,true)
POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,__int8,true)
POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned __int16,true)
POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,__int16,true)
POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned __int32,true)
POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,__int32,true)
#ifdef __BORLANDC__
POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned __int64,true)
POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,__int64,true)
#endif
#endif

# if defined(POTHOS_HAS_LONG_LONG)
POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral, ::Pothos::ulong_long_type,true)
POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral, ::Pothos::long_long_type,true)
#elif defined(POTHOS_HAS_MS_INT64)
POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned __int64,true)
POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,__int64,true)
#endif
        
#ifdef POTHOS_HAS_INT128
POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,Pothos::int128_type,true)
POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,Pothos::uint128_type,true)
#endif
#ifndef POTHOS_NO_CXX11_CHAR16_T
POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,char16_t,true)
#endif
#ifndef POTHOS_NO_CXX11_CHAR32_T
POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,char32_t,true)
#endif

#endif  // non-CodeGear implementation

} // namespace boost

#include <Pothos/serialization/impl/type_traits/detail/bool_trait_undef.hpp>

#endif // BOOST_TT_IS_INTEGRAL_HPP_INCLUDED
