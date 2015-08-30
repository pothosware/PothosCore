//  (C) Copyright John Maddock 2001 - 2003.
//  (C) Copyright Jens Maurer 2001 - 2003.
//  (C) Copyright Aleksey Gurtovoy 2002.
//  (C) Copyright David Abrahams 2002 - 2003.
//  (C) Copyright Toon Knapen 2003.
//  (C) Copyright Boris Gubenko 2006 - 2007.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  HP aCC C++ compiler setup:

#if defined(__EDG__)
#include "Pothos/serialization/impl/config/compiler/common_edg.hpp"
#endif

#if (__HP_aCC <= 33100)
#    define POTHOS_NO_INTEGRAL_INT64_T
#    define POTHOS_NO_OPERATORS_IN_NAMESPACE
#  if !defined(_NAMESPACE_STD)
#     define POTHOS_NO_STD_LOCALE
#     define POTHOS_NO_STRINGSTREAM
#  endif
#endif

#if (__HP_aCC <= 33300)
// member templates are sufficiently broken that we disable them for now
#    define POTHOS_NO_MEMBER_TEMPLATES
#    define POTHOS_NO_DEPENDENT_NESTED_DERIVATIONS
#    define POTHOS_NO_USING_DECLARATION_OVERLOADS_FROM_TYPENAME_BASE
#endif

#if (__HP_aCC <= 38000)
#  define POTHOS_NO_TWO_PHASE_NAME_LOOKUP
#endif

#if (__HP_aCC > 50000) && (__HP_aCC < 60000)
#    define POTHOS_NO_UNREACHABLE_RETURN_DETECTION
#    define POTHOS_NO_TEMPLATE_TEMPLATES
#    define POTHOS_NO_SWPRINTF
#    define POTHOS_NO_DEPENDENT_TYPES_IN_TEMPLATE_VALUE_PARAMETERS
#    define POTHOS_NO_IS_ABSTRACT
#    define POTHOS_NO_MEMBER_TEMPLATE_FRIENDS
#endif

// optional features rather than defects:
#if (__HP_aCC >= 33900)
#    define POTHOS_HAS_LONG_LONG
#    define POTHOS_HAS_PARTIAL_STD_ALLOCATOR
#endif

#if (__HP_aCC >= 50000 ) && (__HP_aCC <= 53800 ) || (__HP_aCC < 31300 )
#    define POTHOS_NO_MEMBER_TEMPLATE_KEYWORD
#endif

// This macro should not be defined when compiling in strict ansi
// mode, but, currently, we don't have the ability to determine
// what standard mode we are compiling with. Some future version
// of aCC6 compiler will provide predefined macros reflecting the
// compilation options, including the standard mode.
#if (__HP_aCC >= 60000) || ((__HP_aCC > 38000) && defined(__hpxstd98))
#    define POTHOS_NO_TWO_PHASE_NAME_LOOKUP
#endif

#define POTHOS_COMPILER "HP aCC version " POTHOS_STRINGIZE(__HP_aCC)

//
// versions check:
// we don't support HP aCC prior to version 33000:
#if __HP_aCC < 33000
#  error "Compiler not supported or configured - please reconfigure"
#endif

//
// Extended checks for supporting aCC on PA-RISC
#if __HP_aCC > 30000 && __HP_aCC < 50000
#  if __HP_aCC < 38000
      // versions prior to version A.03.80 not supported
#     error "Compiler version not supported - version A.03.80 or higher is required"
#  elif !defined(__hpxstd98)
      // must compile using the option +hpxstd98 with version A.03.80 and above
#     error "Compiler option '+hpxstd98' is required for proper support"
#  endif //PA-RISC
#endif

//
// C++0x features
//
//   See boost\config\suffix.hpp for BOOST_NO_LONG_LONG
//
#if !defined(__EDG__)

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
#define POTHOS_NO_CXX11_USER_DEFINED_LITERALS
#define POTHOS_NO_CXX11_ALIGNAS
#define POTHOS_NO_CXX11_TRAILING_RESULT_TYPES
#define POTHOS_NO_CXX11_INLINE_NAMESPACES

/*
  See https://forums13.itrc.hp.com/service/forums/questionanswer.do?threadId=1443331 and
      https://forums13.itrc.hp.com/service/forums/questionanswer.do?threadId=1443436
*/

#if (__HP_aCC < 62500) || !defined(HP_CXX0x_SOURCE)
  #define POTHOS_NO_CXX11_VARIADIC_MACROS
#endif

#endif

//
// last known and checked version for HP-UX/ia64 is 61300
// last known and checked version for PA-RISC is 38000
#if ((__HP_aCC > 61300) || ((__HP_aCC > 38000) && defined(__hpxstd98)))
#  if defined(POTHOS_ASSERT_CONFIG)
#     error "Unknown compiler version - please run the configure tests and report the results"
#  endif
#endif
