//  (C) Copyright John Maddock 2001 - 2002.
//  (C) Copyright Jens Maurer 2001.
//  (C) Copyright David Abrahams 2002.
//  (C) Copyright Aleksey Gurtovoy 2002.
//  (C) Copyright Markus Schoepflin 2005.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//
// Options common to all edg based compilers.
//
// This is included from within the individual compiler mini-configs.

#ifndef  __EDG_VERSION__
#  error This file requires that __EDG_VERSION__ be defined.
#endif

#if (__EDG_VERSION__ <= 238)
#   define POTHOS_NO_INTEGRAL_INT64_T
#   define POTHOS_NO_SFINAE
#endif

#if (__EDG_VERSION__ <= 240)
#   define POTHOS_NO_VOID_RETURNS
#endif

#if (__EDG_VERSION__ <= 241) && !defined(POTHOS_NO_ARGUMENT_DEPENDENT_LOOKUP)
#   define POTHOS_NO_ARGUMENT_DEPENDENT_LOOKUP
#endif

#if (__EDG_VERSION__ <= 244) && !defined(POTHOS_NO_TEMPLATE_TEMPLATES)
#   define POTHOS_NO_TEMPLATE_TEMPLATES
#endif

#if (__EDG_VERSION__ < 300) && !defined(POTHOS_NO_IS_ABSTRACT)
#   define POTHOS_NO_IS_ABSTRACT
#endif

#if (__EDG_VERSION__ <= 303) && !defined(POTHOS_FUNCTION_SCOPE_USING_DECLARATION_BREAKS_ADL)
#   define POTHOS_FUNCTION_SCOPE_USING_DECLARATION_BREAKS_ADL
#endif

// See also kai.hpp which checks a Kai-specific symbol for EH
# if !defined(__KCC) && !defined(__EXCEPTIONS) && !defined(POTHOS_NO_EXCEPTIONS)
#     define POTHOS_NO_EXCEPTIONS
# endif

# if !defined(__NO_LONG_LONG)
#     define POTHOS_HAS_LONG_LONG
# else
#     define POTHOS_NO_LONG_LONG
# endif

// Not sure what version was the first to support #pragma once, but
// different EDG-based compilers (e.g. Intel) supported it for ages.
// Add a proper version check if it causes problems.
#define POTHOS_HAS_PRAGMA_ONCE

//
// C++0x features
//
//   See above for BOOST_NO_LONG_LONG
//
#if (__EDG_VERSION__ < 310)
#  define POTHOS_NO_CXX11_EXTERN_TEMPLATE
#endif
#if (__EDG_VERSION__ <= 310)
// No support for initializer lists
#  define POTHOS_NO_CXX11_HDR_INITIALIZER_LIST
#endif
#if (__EDG_VERSION__ < 400)
#  define POTHOS_NO_CXX11_VARIADIC_MACROS
#endif

#define POTHOS_NO_CXX11_AUTO_DECLARATIONS
#define POTHOS_NO_CXX11_AUTO_MULTIDECLARATIONS
#define POTHOS_NO_CXX11_CHAR16_T
#define POTHOS_NO_CXX11_CHAR32_T
#define POTHOS_NO_CXX11_CONSTEXPR
#define POTHOS_NO_CXX11_DECLTYPE
#define POTHOS_NO_CXX11_DECLTYPE_N3276
#define POTHOS_NO_CXX11_DEFAULTED_FUNCTIONS
#define POTHOS_NO_CXX11_DELETED_FUNCTIONS
#define POTHOS_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS
#define POTHOS_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
#define POTHOS_NO_CXX11_LAMBDAS
#define POTHOS_NO_CXX11_LOCAL_CLASS_TEMPLATE_PARAMETERS
#define POTHOS_NO_CXX11_NOEXCEPT
#define POTHOS_NO_CXX11_NULLPTR
#define POTHOS_NO_CXX11_RANGE_BASED_FOR
#define POTHOS_NO_CXX11_RAW_LITERALS
#define POTHOS_NO_CXX11_RVALUE_REFERENCES
#define POTHOS_NO_CXX11_SCOPED_ENUMS
#define POTHOS_NO_SFINAE_EXPR
#define POTHOS_NO_CXX11_STATIC_ASSERT
#define POTHOS_NO_CXX11_TEMPLATE_ALIASES
#define POTHOS_NO_CXX11_UNICODE_LITERALS
#define POTHOS_NO_CXX11_VARIADIC_TEMPLATES
#define POTHOS_NO_CXX11_UNIFIED_INITIALIZATION_SYNTAX
#define POTHOS_NO_CXX11_USER_DEFINED_LITERALS
#define POTHOS_NO_CXX11_ALIGNAS
#define POTHOS_NO_CXX11_TRAILING_RESULT_TYPES
#define POTHOS_NO_CXX11_INLINE_NAMESPACES

#ifdef c_plusplus
// EDG has "long long" in non-strict mode
// However, some libraries have insufficient "long long" support
// #define BOOST_HAS_LONG_LONG
#endif
