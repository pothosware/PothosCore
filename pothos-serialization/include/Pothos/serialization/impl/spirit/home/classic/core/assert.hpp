/*=============================================================================
    Copyright (c) 2001-2003 Joel de Guzman
    Copyright (c) 2002-2003 Hartmut Kaiser
    http://spirit.sourceforge.net/

  Distributed under the Boost Software License, Version 1.0. (See accompanying
  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(POTHOS_SPIRIT_ASSERT_HPP)
#define POTHOS_SPIRIT_ASSERT_HPP

#include <Pothos/serialization/impl/config.hpp>
#include <Pothos/serialization/impl/throw_exception.hpp>

///////////////////////////////////////////////////////////////////////////////
//
//  BOOST_SPIRIT_ASSERT is used throughout the framework.  It can be
//  overridden by the user. If BOOST_SPIRIT_ASSERT_EXCEPTION is defined,
//  then that will be thrown, otherwise, BOOST_SPIRIT_ASSERT simply turns
//  into a plain BOOST_ASSERT()
//
///////////////////////////////////////////////////////////////////////////////
#if !defined(POTHOS_SPIRIT_ASSERT)
#if defined(NDEBUG)
    #define POTHOS_SPIRIT_ASSERT(x)
#elif defined (POTHOS_SPIRIT_ASSERT_EXCEPTION)
    #define POTHOS_SPIRIT_ASSERT_AUX(f, l, x) POTHOS_SPIRIT_ASSERT_AUX2(f, l, x)
    #define POTHOS_SPIRIT_ASSERT_AUX2(f, l, x)                                   \
    do{ if (!(x)) Pothos::throw_exception(                                       \
        POTHOS_SPIRIT_ASSERT_EXCEPTION(f "(" #l "): " #x)); } while(0)
    #define POTHOS_SPIRIT_ASSERT(x) POTHOS_SPIRIT_ASSERT_AUX(__FILE__, __LINE__, x)
#else
    #include <Pothos/serialization/impl/assert.hpp>
    #define POTHOS_SPIRIT_ASSERT(x) POTHOS_ASSERT(x)
#endif
#endif // !defined(BOOST_SPIRIT_ASSERT)

#endif // BOOST_SPIRIT_ASSERT_HPP
