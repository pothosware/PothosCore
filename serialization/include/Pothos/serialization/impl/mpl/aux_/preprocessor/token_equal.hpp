
#ifndef POTHOS_MPL_AUX_PREPROCESSOR_TOKEN_EQUAL_HPP_INCLUDED
#define POTHOS_MPL_AUX_PREPROCESSOR_TOKEN_EQUAL_HPP_INCLUDED

// Copyright Paul Mensonides 2003
// Copyright Aleksey Gurtovoy 2003-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: token_equal.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/aux_/preprocessor/is_seq.hpp>

#include <Pothos/serialization/impl/preprocessor/if.hpp>
#include <Pothos/serialization/impl/preprocessor/logical/bitand.hpp>
#include <Pothos/serialization/impl/preprocessor/logical/compl.hpp>
#include <Pothos/serialization/impl/preprocessor/tuple/eat.hpp>
#include <Pothos/serialization/impl/preprocessor/cat.hpp>

// compares tokens 'a' and 'b' for equality:
//
//   #define BOOST_MPL_PP_TOKEN_EQUAL_apple(x) x
//   #define BOOST_MPL_PP_TOKEN_EQUAL_orange(x) x
//
//   BOOST_PP_ASSERT( BOOST_PP_NOT( BOOST_MPL_PP_TOKEN_EQUAL(apple, abc) ) )
//   BOOST_PP_ASSERT( BOOST_PP_NOT( BOOST_MPL_PP_TOKEN_EQUAL(abc, apple) ) )
//   BOOST_PP_ASSERT( BOOST_PP_NOT( BOOST_MPL_PP_TOKEN_EQUAL(apple, orange) ) )
//   BOOST_PP_ASSERT( BOOST_MPL_PP_TOKEN_EQUAL(apple, apple) )
//   BOOST_PP_ASSERT( BOOST_MPL_PP_TOKEN_EQUAL(orange, orange) )

#define POTHOS_MPL_PP_TOKEN_EQUAL(a, b) \
    POTHOS_PP_IIF( \
        POTHOS_PP_BITAND( \
              POTHOS_MPL_PP_IS_SEQ( POTHOS_PP_CAT(POTHOS_MPL_PP_TOKEN_EQUAL_, a)((unused)) ) \
            , POTHOS_MPL_PP_IS_SEQ( POTHOS_PP_CAT(POTHOS_MPL_PP_TOKEN_EQUAL_, b)((unused)) ) \
            ) \
        , POTHOS_MPL_PP_TOKEN_EQUAL_I \
        , 0 POTHOS_PP_TUPLE_EAT(2) \
        )(a, b) \
/**/

#define POTHOS_MPL_PP_TOKEN_EQUAL_I(a, b) \
    POTHOS_PP_COMPL(POTHOS_MPL_PP_IS_SEQ( \
        POTHOS_MPL_PP_TOKEN_EQUAL_ ## a( \
            POTHOS_MPL_PP_TOKEN_EQUAL_ ## b \
            )((unused)) \
        )) \
/**/

#endif // BOOST_MPL_AUX_PREPROCESSOR_TOKEN_EQUAL_HPP_INCLUDED
