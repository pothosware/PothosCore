
//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef POTHOS_TT_CONFIG_HPP_INCLUDED
#define POTHOS_TT_CONFIG_HPP_INCLUDED

#ifndef POTHOS_CONFIG_HPP
#include <Pothos/serialization/impl/config.hpp>
#endif

#include <Pothos/serialization/impl/detail/workaround.hpp>

//
// whenever we have a conversion function with ellipses
// it needs to be declared __cdecl to suppress compiler
// warnings from MS and Borland compilers (this *must*
// appear before we include is_same.hpp below):
#if defined(POTHOS_MSVC) || (defined(__BORLANDC__) && !defined(POTHOS_DISABLE_WIN32))
#   define POTHOS_TT_DECL __cdecl
#else
#   define POTHOS_TT_DECL /**/
#endif

# if (POTHOS_WORKAROUND(__MWERKS__, < 0x3000)                         \
    || POTHOS_WORKAROUND(POTHOS_MSVC, <= 1301)                        \
    || !defined(__EDG_VERSION__) && POTHOS_WORKAROUND(__GNUC__, < 3) \
    || POTHOS_WORKAROUND(__IBMCPP__, < 600 )                         \
    || POTHOS_WORKAROUND(__BORLANDC__, < 0x5A0)                      \
    || defined(__ghs)                                               \
    || POTHOS_WORKAROUND(__HP_aCC, < 60700)           \
    || POTHOS_WORKAROUND(MPW_CPLUS, POTHOS_TESTED_AT(0x890))          \
    || POTHOS_WORKAROUND(__SUNPRO_CC, POTHOS_TESTED_AT(0x580)))       \
    && defined(POTHOS_NO_IS_ABSTRACT)

#   define POTHOS_TT_NO_CONFORMING_IS_CLASS_IMPLEMENTATION 1

#endif

#ifndef POTHOS_TT_NO_CONFORMING_IS_CLASS_IMPLEMENTATION
# define POTHOS_TT_HAS_CONFORMING_IS_CLASS_IMPLEMENTATION 1
#endif

//
// Define BOOST_TT_NO_ELLIPSIS_IN_FUNC_TESTING 
// when we can't test for function types with elipsis:
//
#if POTHOS_WORKAROUND(__GNUC__, < 3)
#  define POTHOS_TT_NO_ELLIPSIS_IN_FUNC_TESTING
#endif

//
// define BOOST_TT_TEST_MS_FUNC_SIGS
// when we want to test __stdcall etc function types with is_function etc
// (Note, does not work with Borland, even though it does support __stdcall etc):
//
#if defined(_MSC_EXTENSIONS) && !defined(__BORLANDC__)
#  define POTHOS_TT_TEST_MS_FUNC_SIGS
#endif

//
// define BOOST_TT_NO_CV_FUNC_TEST
// if tests for cv-qualified member functions don't 
// work in is_member_function_pointer
//
#if POTHOS_WORKAROUND(__MWERKS__, < 0x3000) || POTHOS_WORKAROUND(__IBMCPP__, <= 600)
#  define POTHOS_TT_NO_CV_FUNC_TEST
#endif

#endif // BOOST_TT_CONFIG_HPP_INCLUDED


