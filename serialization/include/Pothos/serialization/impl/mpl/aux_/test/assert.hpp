
#ifndef POTHOS_MPL_AUX_TEST_ASSERT_HPP_INCLUDED
#define POTHOS_MPL_AUX_TEST_ASSERT_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2002-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: assert.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/assert.hpp>
#include <Pothos/serialization/impl/preprocessor/cat.hpp>

#define MPL_ASSERT(pred)                POTHOS_MPL_ASSERT(pred)
#define MPL_ASSERT_NOT(pred)            POTHOS_MPL_ASSERT_NOT(pred)
#define MPL_ASSERT_MSG(c, msg, types)   POTHOS_MPL_ASSERT_MSG(c, msg, types)
#define MPL_ASSERT_RELATION(x, rel, y)  POTHOS_MPL_ASSERT_RELATION(x, rel, y)

#define MPL_ASSERT_INSTANTIATION(x) \
    enum { POTHOS_PP_CAT(instantiation_test, __LINE__) = sizeof( x ) } \
/**/

#endif // BOOST_MPL_AUX_TEST_ASSERT_HPP_INCLUDED
