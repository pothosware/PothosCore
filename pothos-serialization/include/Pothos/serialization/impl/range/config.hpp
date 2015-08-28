// Boost.Range library
//
//  Copyright Thorsten Ottosen 2003-2004. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//

#ifndef POTHOS_RANGE_CONFIG_HPP
#define POTHOS_RANGE_CONFIG_HPP

#include <Pothos/serialization/impl/detail/workaround.hpp>

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif

#include <Pothos/serialization/impl/config.hpp>

#ifdef POTHOS_RANGE_DEDUCED_TYPENAME
#error "macro already defined!"
#endif

#if POTHOS_WORKAROUND(__BORLANDC__, POTHOS_TESTED_AT(0x564))
# define POTHOS_RANGE_DEDUCED_TYPENAME typename
#else
# if POTHOS_WORKAROUND(POTHOS_MSVC, == 1300) && !defined(_MSC_EXTENSIONS)
#  define POTHOS_RANGE_DEDUCED_TYPENAME typename
# else
#  define POTHOS_RANGE_DEDUCED_TYPENAME POTHOS_DEDUCED_TYPENAME
# endif
#endif

#ifdef POTHOS_RANGE_NO_ARRAY_SUPPORT
#error "macro already defined!"
#endif

#if POTHOS_WORKAROUND( POTHOS_MSVC, < 1300 ) || POTHOS_WORKAROUND( __MWERKS__, <= 0x3003 )
#define POTHOS_RANGE_NO_ARRAY_SUPPORT 1
#endif

#ifdef POTHOS_RANGE_NO_ARRAY_SUPPORT
#define POTHOS_RANGE_ARRAY_REF() (Pothos_range_array)
#define POTHOS_RANGE_NO_STATIC_ASSERT
#else
#define POTHOS_RANGE_ARRAY_REF() (&Pothos_range_array)
#endif



#endif

