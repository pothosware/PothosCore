
#ifndef POTHOS_MPL_AUX_PREPROCESSOR_EXT_PARAMS_HPP_INCLUDED
#define POTHOS_MPL_AUX_PREPROCESSOR_EXT_PARAMS_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: ext_params.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/aux_/config/preprocessor.hpp>

// BOOST_MPL_PP_EXT_PARAMS(2,2,T): <nothing>
// BOOST_MPL_PP_EXT_PARAMS(2,3,T): T2
// BOOST_MPL_PP_EXT_PARAMS(2,4,T): T2, T3
// BOOST_MPL_PP_EXT_PARAMS(2,n,T): T2, T3, .., Tn-1

#if !defined(POTHOS_MPL_CFG_NO_OWN_PP_PRIMITIVES)

#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/filter_params.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/sub.hpp>

#   define POTHOS_MPL_PP_EXT_PARAMS(i,j,p) \
    POTHOS_MPL_PP_EXT_PARAMS_DELAY_1(i,POTHOS_MPL_PP_SUB(j,i),p) \
    /**/

#   define POTHOS_MPL_PP_EXT_PARAMS_DELAY_1(i,n,p) \
    POTHOS_MPL_PP_EXT_PARAMS_DELAY_2(i,n,p) \
    /**/

#   define POTHOS_MPL_PP_EXT_PARAMS_DELAY_2(i,n,p) \
    POTHOS_MPL_PP_EXT_PARAMS_##i(n,p) \
    /**/

#   define POTHOS_MPL_PP_EXT_PARAMS_1(i,p) POTHOS_MPL_PP_FILTER_PARAMS_##i(p##1,p##2,p##3,p##4,p##5,p##6,p##7,p##8,p##9)
#   define POTHOS_MPL_PP_EXT_PARAMS_2(i,p) POTHOS_MPL_PP_FILTER_PARAMS_##i(p##2,p##3,p##4,p##5,p##6,p##7,p##8,p##9,p1)
#   define POTHOS_MPL_PP_EXT_PARAMS_3(i,p) POTHOS_MPL_PP_FILTER_PARAMS_##i(p##3,p##4,p##5,p##6,p##7,p##8,p##9,p1,p2)
#   define POTHOS_MPL_PP_EXT_PARAMS_4(i,p) POTHOS_MPL_PP_FILTER_PARAMS_##i(p##4,p##5,p##6,p##7,p##8,p##9,p1,p2,p3)
#   define POTHOS_MPL_PP_EXT_PARAMS_5(i,p) POTHOS_MPL_PP_FILTER_PARAMS_##i(p##5,p##6,p##7,p##8,p##9,p1,p2,p3,p4)
#   define POTHOS_MPL_PP_EXT_PARAMS_6(i,p) POTHOS_MPL_PP_FILTER_PARAMS_##i(p##6,p##7,p##8,p##9,p1,p2,p3,p4,p5)
#   define POTHOS_MPL_PP_EXT_PARAMS_7(i,p) POTHOS_MPL_PP_FILTER_PARAMS_##i(p##7,p##8,p##9,p1,p2,p3,p4,p5,p6)
#   define POTHOS_MPL_PP_EXT_PARAMS_8(i,p) POTHOS_MPL_PP_FILTER_PARAMS_##i(p##8,p##9,p1,p2,p3,p4,p5,p6,p7)
#   define POTHOS_MPL_PP_EXT_PARAMS_9(i,p) POTHOS_MPL_PP_FILTER_PARAMS_##i(p##9,p1,p2,p3,p4,p5,p6,p7,p8)

#else

#   include <Pothos/serialization/impl/preprocessor/arithmetic/add.hpp>
#   include <Pothos/serialization/impl/preprocessor/arithmetic/sub.hpp>
#   include <Pothos/serialization/impl/preprocessor/comma_if.hpp>
#   include <Pothos/serialization/impl/preprocessor/repeat.hpp>
#   include <Pothos/serialization/impl/preprocessor/tuple/elem.hpp>
#   include <Pothos/serialization/impl/preprocessor/cat.hpp>

#   define POTHOS_MPL_PP_AUX_EXT_PARAM_FUNC(unused, i, op) \
    POTHOS_PP_COMMA_IF(i) \
    POTHOS_PP_CAT( \
          POTHOS_PP_TUPLE_ELEM(2,1,op) \
        , POTHOS_PP_ADD_D(1, i, POTHOS_PP_TUPLE_ELEM(2,0,op)) \
        ) \
    /**/

#   define POTHOS_MPL_PP_EXT_PARAMS(i, j, param) \
    POTHOS_PP_REPEAT( \
          POTHOS_PP_SUB_D(1,j,i) \
        , POTHOS_MPL_PP_AUX_EXT_PARAM_FUNC \
        , (i,param) \
        ) \
    /**/

#endif

#endif // BOOST_MPL_AUX_PREPROCESSOR_EXT_PARAMS_HPP_INCLUDED
