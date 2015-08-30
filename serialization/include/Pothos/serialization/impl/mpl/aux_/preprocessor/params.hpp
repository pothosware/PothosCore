
#ifndef POTHOS_MPL_AUX_PREPROCESSOR_PARAMS_HPP_INCLUDED
#define POTHOS_MPL_AUX_PREPROCESSOR_PARAMS_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: params.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/aux_/config/preprocessor.hpp>

// BOOST_MPL_PP_PARAMS(0,T): <nothing>
// BOOST_MPL_PP_PARAMS(1,T): T1
// BOOST_MPL_PP_PARAMS(2,T): T1, T2
// BOOST_MPL_PP_PARAMS(n,T): T1, T2, .., Tn

#if !defined(POTHOS_MPL_CFG_NO_OWN_PP_PRIMITIVES)

#   include <Pothos/serialization/impl/preprocessor/cat.hpp>

#   define POTHOS_MPL_PP_PARAMS(n,p) \
    POTHOS_PP_CAT(POTHOS_MPL_PP_PARAMS_,n)(p) \
    /**/

#   define POTHOS_MPL_PP_PARAMS_0(p)
#   define POTHOS_MPL_PP_PARAMS_1(p) p##1
#   define POTHOS_MPL_PP_PARAMS_2(p) p##1,p##2
#   define POTHOS_MPL_PP_PARAMS_3(p) p##1,p##2,p##3
#   define POTHOS_MPL_PP_PARAMS_4(p) p##1,p##2,p##3,p##4
#   define POTHOS_MPL_PP_PARAMS_5(p) p##1,p##2,p##3,p##4,p##5
#   define POTHOS_MPL_PP_PARAMS_6(p) p##1,p##2,p##3,p##4,p##5,p##6
#   define POTHOS_MPL_PP_PARAMS_7(p) p##1,p##2,p##3,p##4,p##5,p##6,p##7
#   define POTHOS_MPL_PP_PARAMS_8(p) p##1,p##2,p##3,p##4,p##5,p##6,p##7,p##8
#   define POTHOS_MPL_PP_PARAMS_9(p) p##1,p##2,p##3,p##4,p##5,p##6,p##7,p##8,p##9

#else

#   include <Pothos/serialization/impl/preprocessor/comma_if.hpp>
#   include <Pothos/serialization/impl/preprocessor/repeat.hpp>
#   include <Pothos/serialization/impl/preprocessor/inc.hpp>
#   include <Pothos/serialization/impl/preprocessor/cat.hpp>

#   define POTHOS_MPL_PP_AUX_PARAM_FUNC(unused, i, param) \
    POTHOS_PP_COMMA_IF(i) \
    POTHOS_PP_CAT(param, POTHOS_PP_INC(i)) \
    /**/

#   define POTHOS_MPL_PP_PARAMS(n, param) \
    POTHOS_PP_REPEAT( \
          n \
        , POTHOS_MPL_PP_AUX_PARAM_FUNC \
        , param \
        ) \
    /**/

#endif 

#endif // BOOST_MPL_AUX_PREPROCESSOR_PARAMS_HPP_INCLUDED
