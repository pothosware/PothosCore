//  (C) Copyright John Maddock 2001.
//  (C) Copyright Darin Adler 2001.
//  (C) Copyright Peter Dimov 2001.
//  (C) Copyright David Abrahams 2001 - 2002.
//  (C) Copyright Beman Dawes 2001 - 2003.
//  (C) Copyright Stefan Slapeta 2004.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  Metrowerks C++ compiler setup:

// locale support is disabled when linking with the dynamic runtime
#   ifdef _MSL_NO_LOCALE
#     define POTHOS_NO_STD_LOCALE
#   endif

#   if __MWERKS__ <= 0x2301  // 5.3
#     define POTHOS_NO_FUNCTION_TEMPLATE_ORDERING
#     define POTHOS_NO_POINTER_TO_MEMBER_CONST
#     define POTHOS_NO_DEPENDENT_TYPES_IN_TEMPLATE_VALUE_PARAMETERS
#     define POTHOS_NO_MEMBER_TEMPLATE_KEYWORD
#   endif

#   if __MWERKS__ <= 0x2401  // 6.2
//#     define BOOST_NO_FUNCTION_TEMPLATE_ORDERING
#   endif

#   if(__MWERKS__ <= 0x2407)  // 7.x
#     define POTHOS_NO_MEMBER_FUNCTION_SPECIALIZATIONS
#     define POTHOS_NO_UNREACHABLE_RETURN_DETECTION
#   endif

#   if(__MWERKS__ <= 0x3003)  // 8.x
#     define POTHOS_NO_SFINAE
#    endif

// the "|| !defined(BOOST_STRICT_CONFIG)" part should apply to the last
// tested version *only*:
#   if(__MWERKS__ <= 0x3207) || !defined(POTHOS_STRICT_CONFIG) // 9.6
#     define POTHOS_NO_MEMBER_TEMPLATE_FRIENDS
#     define POTHOS_NO_IS_ABSTRACT
#    endif

#if !__option(wchar_type)
#   define POTHOS_NO_INTRINSIC_WCHAR_T
#endif

#if !__option(exceptions) && !defined(POTHOS_NO_EXCEPTIONS)
#   define POTHOS_NO_EXCEPTIONS
#endif

#if (__INTEL__ && _WIN32) || (__POWERPC__ && macintosh)
#   if __MWERKS__ == 0x3000
#     define POTHOS_COMPILER_VERSION 8.0
#   elif __MWERKS__ == 0x3001
#     define POTHOS_COMPILER_VERSION 8.1
#   elif __MWERKS__ == 0x3002
#     define POTHOS_COMPILER_VERSION 8.2
#   elif __MWERKS__ == 0x3003
#     define POTHOS_COMPILER_VERSION 8.3
#   elif __MWERKS__ == 0x3200
#     define POTHOS_COMPILER_VERSION 9.0
#   elif __MWERKS__ == 0x3201
#     define POTHOS_COMPILER_VERSION 9.1
#   elif __MWERKS__ == 0x3202
#     define POTHOS_COMPILER_VERSION 9.2
#   elif __MWERKS__ == 0x3204
#     define POTHOS_COMPILER_VERSION 9.3
#   elif __MWERKS__ == 0x3205
#     define POTHOS_COMPILER_VERSION 9.4
#   elif __MWERKS__ == 0x3206
#     define POTHOS_COMPILER_VERSION 9.5
#   elif __MWERKS__ == 0x3207
#     define POTHOS_COMPILER_VERSION 9.6
#   else
#     define POTHOS_COMPILER_VERSION __MWERKS__
#   endif
#else
#  define POTHOS_COMPILER_VERSION __MWERKS__
#endif

//
// C++0x features
//
//   See boost\config\suffix.hpp for BOOST_NO_LONG_LONG
//
#if __MWERKS__ > 0x3206 && __option(rvalue_refs)
#  define POTHOS_HAS_RVALUE_REFS
#else
#  define POTHOS_NO_CXX11_RVALUE_REFERENCES
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
#define POTHOS_NO_CXX11_EXTERN_TEMPLATE
#define POTHOS_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
#define POTHOS_NO_CXX11_HDR_INITIALIZER_LIST
#define POTHOS_NO_CXX11_LAMBDAS
#define POTHOS_NO_CXX11_LOCAL_CLASS_TEMPLATE_PARAMETERS
#define POTHOS_NO_CXX11_NOEXCEPT
#define POTHOS_NO_CXX11_NULLPTR
#define POTHOS_NO_CXX11_RANGE_BASED_FOR
#define POTHOS_NO_CXX11_RAW_LITERALS
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

#define POTHOS_COMPILER "Metrowerks CodeWarrior C++ version " POTHOS_STRINGIZE(POTHOS_COMPILER_VERSION)

//
// versions check:
// we don't support Metrowerks prior to version 5.3:
#if __MWERKS__ < 0x2301
#  error "Compiler not supported or configured - please reconfigure"
#endif
//
// last known and checked version:
#if (__MWERKS__ > 0x3205)
#  if defined(POTHOS_ASSERT_CONFIG)
#     error "Unknown compiler version - please run the configure tests and report the results"
#  endif
#endif







