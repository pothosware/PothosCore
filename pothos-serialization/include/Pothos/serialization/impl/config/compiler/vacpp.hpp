//  (C) Copyright John Maddock 2001 - 2003.
//  (C) Copyright Toon Knapen 2001 - 2003.
//  (C) Copyright Lie-Quan Lee 2001.
//  (C) Copyright Markus Schoepflin 2002 - 2003.
//  (C) Copyright Beman Dawes 2002 - 2003.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  Visual Age (IBM) C++ compiler setup:

#if __IBMCPP__ <= 501
#  define POTHOS_NO_MEMBER_TEMPLATE_FRIENDS
#  define POTHOS_NO_MEMBER_FUNCTION_SPECIALIZATIONS
#endif

#if (__IBMCPP__ <= 502)
// Actually the compiler supports inclass member initialization but it
// requires a definition for the class member and it doesn't recognize
// it as an integral constant expression when used as a template argument.
#  define POTHOS_NO_INCLASS_MEMBER_INITIALIZATION
#  define POTHOS_NO_INTEGRAL_INT64_T
#  define POTHOS_NO_MEMBER_TEMPLATE_KEYWORD
#endif

#if (__IBMCPP__ <= 600) || !defined(POTHOS_STRICT_CONFIG)
#  define POTHOS_NO_POINTER_TO_MEMBER_TEMPLATE_PARAMETERS
#endif

#if (__IBMCPP__ <= 1110)
// XL C++ V11.1 and earlier versions may not always value-initialize
// a temporary object T(), when T is a non-POD aggregate class type.
// Michael Wong (IBM Canada Ltd) has confirmed this issue and gave it
// high priority. -- Niels Dekker (LKEB), May 2010.
#  define POTHOS_NO_COMPLETE_VALUE_INITIALIZATION
#endif

//
// On AIX thread support seems to be indicated by _THREAD_SAFE:
//
#ifdef _THREAD_SAFE
#  define POTHOS_HAS_THREADS
#endif

#define POTHOS_COMPILER "IBM Visual Age version " POTHOS_STRINGIZE(__IBMCPP__)

//
// versions check:
// we don't support Visual age prior to version 5:
#if __IBMCPP__ < 500
#error "Compiler not supported or configured - please reconfigure"
#endif
//
// last known and checked version is 1210:
#if (__IBMCPP__ > 1210)
#  if defined(POTHOS_ASSERT_CONFIG)
#     error "Unknown compiler version - please run the configure tests and report the results"
#  endif
#endif

// Some versions of the compiler have issues with default arguments on partial specializations
#if __IBMCPP__ <= 1010
#define POTHOS_NO_PARTIAL_SPECIALIZATION_IMPLICIT_DEFAULT_ARGS
#endif

//
// C++0x features
//
//   See boost\config\suffix.hpp for BOOST_NO_LONG_LONG
//
#if ! __IBMCPP_AUTO_TYPEDEDUCTION
#  define POTHOS_NO_CXX11_AUTO_DECLARATIONS
#  define POTHOS_NO_CXX11_AUTO_MULTIDECLARATIONS
#endif
#if ! __IBMCPP_UTF_LITERAL__
#  define POTHOS_NO_CXX11_CHAR16_T
#  define POTHOS_NO_CXX11_CHAR32_T
#endif
#if ! __IBMCPP_CONSTEXPR
#  define POTHOS_NO_CXX11_CONSTEXPR
#endif
#if ! __IBMCPP_DECLTYPE
#  define POTHOS_NO_CXX11_DECLTYPE
#else
#  define POTHOS_HAS_DECLTYPE
#endif
#define POTHOS_NO_CXX11_DECLTYPE_N3276
#define POTHOS_NO_CXX11_DEFAULTED_FUNCTIONS
#define POTHOS_NO_CXX11_DELETED_FUNCTIONS
#if ! __IBMCPP_EXPLICIT_CONVERSION_OPERATORS
#  define POTHOS_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS
#endif
#if ! __IBMCPP_EXTERN_TEMPLATE
#  define POTHOS_NO_CXX11_EXTERN_TEMPLATE
#endif
#if ! __IBMCPP_VARIADIC_TEMPLATES
// not enabled separately at this time
#  define POTHOS_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
#endif
#define POTHOS_NO_CXX11_HDR_INITIALIZER_LIST
#define POTHOS_NO_CXX11_LAMBDAS
#define POTHOS_NO_CXX11_LOCAL_CLASS_TEMPLATE_PARAMETERS
#define POTHOS_NO_CXX11_NOEXCEPT
#define POTHOS_NO_CXX11_NULLPTR
#define POTHOS_NO_CXX11_RANGE_BASED_FOR
#define POTHOS_NO_CXX11_RAW_LITERALS
#define POTHOS_NO_CXX11_USER_DEFINED_LITERALS
#if ! __IBMCPP_RVALUE_REFERENCES
#  define POTHOS_NO_CXX11_RVALUE_REFERENCES
#endif
#if ! __IBMCPP_SCOPED_ENUM
#  define POTHOS_NO_CXX11_SCOPED_ENUMS
#endif
#define POTHOS_NO_SFINAE_EXPR
#define POTHOS_NO_CXX11_UNIFIED_INITIALIZATION_SYNTAX
#if ! __IBMCPP_STATIC_ASSERT
#  define POTHOS_NO_CXX11_STATIC_ASSERT
#endif
#define POTHOS_NO_CXX11_TEMPLATE_ALIASES
#define POTHOS_NO_CXX11_UNICODE_LITERALS
#if ! __IBMCPP_VARIADIC_TEMPLATES
#  define POTHOS_NO_CXX11_VARIADIC_TEMPLATES
#endif
#if ! __C99_MACRO_WITH_VA_ARGS
#  define POTHOS_NO_CXX11_VARIADIC_MACROS
#endif
#define POTHOS_NO_CXX11_ALIGNAS
#define POTHOS_NO_CXX11_TRAILING_RESULT_TYPES
#define POTHOS_NO_CXX11_INLINE_NAMESPACES
