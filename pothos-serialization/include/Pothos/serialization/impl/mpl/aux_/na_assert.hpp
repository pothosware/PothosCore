
#ifndef POTHOS_MPL_AUX_NA_ASSERT_HPP_INCLUDED
#define POTHOS_MPL_AUX_NA_ASSERT_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2001-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: na_assert.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/aux_/na.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/msvc.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/workaround.hpp>

#if !POTHOS_WORKAROUND(_MSC_FULL_VER, <= 140050601)    \
    && !POTHOS_WORKAROUND(__EDG_VERSION__, <= 243)
#   include <Pothos/serialization/impl/mpl/assert.hpp>
#   define POTHOS_MPL_AUX_ASSERT_NOT_NA(x) \
    POTHOS_MPL_ASSERT_NOT((Pothos::mpl::is_na<type>)) \
/**/
#else
#   include <Pothos/serialization/impl/static_assert.hpp>
#   define POTHOS_MPL_AUX_ASSERT_NOT_NA(x) \
    POTHOS_STATIC_ASSERT(!Pothos::mpl::is_na<x>::value) \
/**/
#endif

#endif // BOOST_MPL_AUX_NA_ASSERT_HPP_INCLUDED
