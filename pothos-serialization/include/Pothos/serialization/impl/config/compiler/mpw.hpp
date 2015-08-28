//  (C) Copyright John Maddock 2001 - 2002.
//  (C) Copyright Aleksey Gurtovoy 2002.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  MPW C++ compilers setup:

#   if    defined(__SC__)
#     define POTHOS_COMPILER "MPW SCpp version " POTHOS_STRINGIZE(__SC__)
#   elif defined(__MRC__)
#     define POTHOS_COMPILER "MPW MrCpp version " POTHOS_STRINGIZE(__MRC__)
#   else
#     error "Using MPW compiler configuration by mistake.  Please update."
#   endif

//
// MPW 8.90:
//
#if (MPW_CPLUS <= 0x890) || !defined(POTHOS_STRICT_CONFIG)
#  define POTHOS_NO_CV_SPECIALIZATIONS
#  define POTHOS_NO_DEPENDENT_NESTED_DERIVATIONS
#  define POTHOS_NO_DEPENDENT_TYPES_IN_TEMPLATE_VALUE_PARAMETERS
#  define POTHOS_NO_INCLASS_MEMBER_INITIALIZATION
#  define POTHOS_NO_INTRINSIC_WCHAR_T
#  define POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION
#  define POTHOS_NO_USING_TEMPLATE

#  define POTHOS_NO_CWCHAR
#  define POTHOS_NO_LIMITS_COMPILE_TIME_CONSTANTS

#  define POTHOS_NO_STD_ALLOCATOR /* actually a bug with const reference overloading */

#endif

//
// C++0x features
//
//   See boost\config\suffix.hpp for BOOST_NO_LONG_LONG
//
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
#define POTHOS_NO_CXX11_EXTERN_TEMPLATE
#define POTHOS_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
#define POTHOS_NO_CXX11_HDR_INITIALIZER_LIST
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
#define POTHOS_NO_CXX11_VARIADIC_MACROS
#define POTHOS_NO_CXX11_UNIFIED_INITIALIZATION_SYNTAX
#define POTHOS_NO_CXX11_USER_DEFINED_LITERALS
#define POTHOS_NO_CXX11_ALIGNAS
#define POTHOS_NO_CXX11_TRAILING_RESULT_TYPES
#define POTHOS_NO_CXX11_INLINE_NAMESPACES

//
// versions check:
// we don't support MPW prior to version 8.9:
#if MPW_CPLUS < 0x890
#  error "Compiler not supported or configured - please reconfigure"
#endif
//
// last known and checked version is 0x890:
#if (MPW_CPLUS > 0x890)
#  if defined(POTHOS_ASSERT_CONFIG)
#     error "Unknown compiler version - please run the configure tests and report the results"
#  endif
#endif


