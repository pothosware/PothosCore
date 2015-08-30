//  (C) Copyright John Maddock 2011.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  Greenhills C compiler setup:

#define POTHOS_COMPILER "Cray C version " POTHOS_STRINGIZE(_RELEASE)

#if _RELEASE < 7
#  error "Boost is not configured for Cray compilers prior to version 7, please try the configure script."
#endif

//
// Check this is a recent EDG based compiler, otherwise we don't support it here:
//
#ifndef __EDG_VERSION__
#  error "Unsupported Cray compiler, please try running the configure script."
#endif

#include "Pothos/serialization/impl/config/compiler/common_edg.hpp"

//
// Cray peculiarities, probably version 7 specific:
//
#undef POTHOS_NO_CXX11_AUTO_DECLARATIONS
#undef POTHOS_NO_CXX11_AUTO_MULTIDECLARATIONS
#define POTHOS_HAS_NRVO
#define POTHOS_NO_CXX11_VARIADIC_TEMPLATES
#define POTHOS_NO_CXX11_UNIFIED_INITIALIZATION_SYNTAX
#define POTHOS_NO_CXX11_UNICODE_LITERALS
#define POTHOS_NO_TWO_PHASE_NAME_LOOKUP
#define POTHOS_HAS_NRVO
#define POTHOS_NO_CXX11_TEMPLATE_ALIASES
#define POTHOS_NO_CXX11_STATIC_ASSERT
#define POTHOS_NO_SFINAE_EXPR
#define POTHOS_NO_CXX11_SCOPED_ENUMS
#define POTHOS_NO_CXX11_RVALUE_REFERENCES
#define POTHOS_NO_CXX11_RANGE_BASED_FOR
#define POTHOS_NO_CXX11_RAW_LITERALS
#define POTHOS_NO_CXX11_NULLPTR
#define POTHOS_NO_CXX11_NOEXCEPT
#define POTHOS_NO_CXX11_LAMBDAS
#define POTHOS_NO_CXX11_LOCAL_CLASS_TEMPLATE_PARAMETERS
#define POTHOS_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
#define POTHOS_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS
#define POTHOS_NO_CXX11_DELETED_FUNCTIONS
#define POTHOS_NO_CXX11_DEFAULTED_FUNCTIONS
#define POTHOS_NO_CXX11_DECLTYPE_N3276
#define POTHOS_NO_CXX11_DECLTYPE
#define POTHOS_NO_CXX11_CONSTEXPR
#define POTHOS_NO_CXX11_USER_DEFINED_LITERALS
#define POTHOS_NO_COMPLETE_VALUE_INITIALIZATION
#define POTHOS_NO_CXX11_CHAR32_T
#define POTHOS_NO_CXX11_CHAR16_T
#define POTHOS_NO_CXX11_ALIGNAS
//#define BOOST_BCB_PARTIAL_SPECIALIZATION_BUG
#define POTHOS_MATH_DISABLE_STD_FPCLASSIFY
//#define BOOST_HAS_FPCLASSIFY

#define POTHOS_SP_USE_PTHREADS
#define POTHOS_AC_USE_PTHREADS

