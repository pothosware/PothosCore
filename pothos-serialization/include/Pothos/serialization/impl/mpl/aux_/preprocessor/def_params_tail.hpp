
#ifndef POTHOS_MPL_AUX_PREPROCESSOR_DEF_PARAMS_TAIL_HPP_INCLUDED
#define POTHOS_MPL_AUX_PREPROCESSOR_DEF_PARAMS_TAIL_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: def_params_tail.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/limits/arity.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/dtp.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/preprocessor.hpp>

#include <Pothos/serialization/impl/preprocessor/comma_if.hpp>
#include <Pothos/serialization/impl/preprocessor/logical/and.hpp>
#include <Pothos/serialization/impl/preprocessor/identity.hpp>
#include <Pothos/serialization/impl/preprocessor/empty.hpp>

// BOOST_MPL_PP_DEF_PARAMS_TAIL(1,T,value): , T1 = value, .., Tn = value
// BOOST_MPL_PP_DEF_PARAMS_TAIL(2,T,value): , T2 = value, .., Tn = value
// BOOST_MPL_PP_DEF_PARAMS_TAIL(n,T,value): <nothing>

#if !defined(POTHOS_MPL_CFG_NO_OWN_PP_PRIMITIVES)

#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/filter_params.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/sub.hpp>

#   define POTHOS_MPL_PP_DEF_PARAMS_TAIL_IMPL(i, param, value_func) \
    POTHOS_MPL_PP_DEF_PARAMS_TAIL_DELAY_1( \
          i \
        , POTHOS_MPL_PP_SUB(POTHOS_MPL_LIMIT_METAFUNCTION_ARITY,i) \
        , param \
        , value_func \
        ) \
    /**/

#   define POTHOS_MPL_PP_DEF_PARAMS_TAIL_DELAY_1(i, n, param, value_func) \
    POTHOS_MPL_PP_DEF_PARAMS_TAIL_DELAY_2(i,n,param,value_func) \
    /**/

#   define POTHOS_MPL_PP_DEF_PARAMS_TAIL_DELAY_2(i, n, param, value_func) \
    POTHOS_PP_COMMA_IF(POTHOS_PP_AND(i,n)) \
    POTHOS_MPL_PP_DEF_PARAMS_TAIL_##i(n,param,value_func) \
    /**/

#   define POTHOS_MPL_PP_DEF_PARAMS_TAIL_0(i,p,v) POTHOS_MPL_PP_FILTER_PARAMS_##i(p##1 v(),p##2 v(),p##3 v(),p##4 v(),p##5 v(),p##6 v(),p##7 v(),p##8 v(),p##9 v())
#   define POTHOS_MPL_PP_DEF_PARAMS_TAIL_1(i,p,v) POTHOS_MPL_PP_FILTER_PARAMS_##i(p##2 v(),p##3 v(),p##4 v(),p##5 v(),p##6 v(),p##7 v(),p##8 v(),p##9 v(),p1)
#   define POTHOS_MPL_PP_DEF_PARAMS_TAIL_2(i,p,v) POTHOS_MPL_PP_FILTER_PARAMS_##i(p##3 v(),p##4 v(),p##5 v(),p##6 v(),p##7 v(),p##8 v(),p##9 v(),p1,p2)
#   define POTHOS_MPL_PP_DEF_PARAMS_TAIL_3(i,p,v) POTHOS_MPL_PP_FILTER_PARAMS_##i(p##4 v(),p##5 v(),p##6 v(),p##7 v(),p##8 v(),p##9 v(),p1,p2,p3)
#   define POTHOS_MPL_PP_DEF_PARAMS_TAIL_4(i,p,v) POTHOS_MPL_PP_FILTER_PARAMS_##i(p##5 v(),p##6 v(),p##7 v(),p##8 v(),p##9 v(),p1,p2,p3,p4)
#   define POTHOS_MPL_PP_DEF_PARAMS_TAIL_5(i,p,v) POTHOS_MPL_PP_FILTER_PARAMS_##i(p##6 v(),p##7 v(),p##8 v(),p##9 v(),p1,p2,p3,p4,p5)
#   define POTHOS_MPL_PP_DEF_PARAMS_TAIL_6(i,p,v) POTHOS_MPL_PP_FILTER_PARAMS_##i(p##7 v(),p##8 v(),p##9 v(),p1,p2,p3,p4,p5,p6)
#   define POTHOS_MPL_PP_DEF_PARAMS_TAIL_7(i,p,v) POTHOS_MPL_PP_FILTER_PARAMS_##i(p##8 v(),p##9 v(),p1,p2,p3,p4,p5,p6,p7)
#   define POTHOS_MPL_PP_DEF_PARAMS_TAIL_8(i,p,v) POTHOS_MPL_PP_FILTER_PARAMS_##i(p##9 v(),p1,p2,p3,p4,p5,p6,p7,p8)
#   define POTHOS_MPL_PP_DEF_PARAMS_TAIL_9(i,p,v) POTHOS_MPL_PP_FILTER_PARAMS_##i(p1,p2,p3,p4,p5,p6,p7,p8,p9)

#else

#   include <Pothos/serialization/impl/preprocessor/arithmetic/add.hpp>
#   include <Pothos/serialization/impl/preprocessor/arithmetic/sub.hpp>
#   include <Pothos/serialization/impl/preprocessor/inc.hpp>
#   include <Pothos/serialization/impl/preprocessor/tuple/elem.hpp>
#   include <Pothos/serialization/impl/preprocessor/repeat.hpp>
#   include <Pothos/serialization/impl/preprocessor/cat.hpp>

#   define POTHOS_MPL_PP_AUX_TAIL_PARAM_FUNC(unused, i, op) \
    , POTHOS_PP_CAT( \
          POTHOS_PP_TUPLE_ELEM(3, 1, op) \
        , POTHOS_PP_ADD_D(1, i, POTHOS_PP_INC(POTHOS_PP_TUPLE_ELEM(3, 0, op))) \
        ) POTHOS_PP_TUPLE_ELEM(3, 2, op)() \
    /**/

#   define POTHOS_MPL_PP_DEF_PARAMS_TAIL_IMPL(i, param, value_func) \
    POTHOS_PP_REPEAT( \
          POTHOS_PP_SUB_D(1, POTHOS_MPL_LIMIT_METAFUNCTION_ARITY, i) \
        , POTHOS_MPL_PP_AUX_TAIL_PARAM_FUNC \
        , (i, param, value_func) \
        ) \
    /**/


#endif // BOOST_MPL_CFG_NO_OWN_PP_PRIMITIVES

#define POTHOS_MPL_PP_DEF_PARAMS_TAIL(i, param, value) \
    POTHOS_MPL_PP_DEF_PARAMS_TAIL_IMPL(i, param, POTHOS_PP_IDENTITY(=value)) \
    /**/

#if !defined(POTHOS_MPL_CFG_NO_DEFAULT_PARAMETERS_IN_NESTED_TEMPLATES)
#   define POTHOS_MPL_PP_NESTED_DEF_PARAMS_TAIL(i, param, value) \
    POTHOS_MPL_PP_DEF_PARAMS_TAIL_IMPL(i, param, POTHOS_PP_IDENTITY(=value)) \
    /**/
#else
#   define POTHOS_MPL_PP_NESTED_DEF_PARAMS_TAIL(i, param, value) \
    POTHOS_MPL_PP_DEF_PARAMS_TAIL_IMPL(i, param, POTHOS_PP_EMPTY) \
    /**/
#endif

#endif // BOOST_MPL_AUX_PREPROCESSOR_DEF_PARAMS_TAIL_HPP_INCLUDED
