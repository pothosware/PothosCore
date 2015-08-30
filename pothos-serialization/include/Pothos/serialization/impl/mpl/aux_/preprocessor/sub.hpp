
#ifndef POTHOS_MPL_AUX_PREPROCESSOR_SUB_HPP_INCLUDED
#define POTHOS_MPL_AUX_PREPROCESSOR_SUB_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2002-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: sub.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/aux_/config/preprocessor.hpp>

#if !defined(POTHOS_MPL_CFG_NO_OWN_PP_PRIMITIVES)

#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/tuple.hpp>

#if defined(POTHOS_MPL_CFG_BROKEN_PP_MACRO_EXPANSION)
#   include <Pothos/serialization/impl/preprocessor/cat.hpp>

#   define POTHOS_MPL_PP_SUB(i,j) \
    POTHOS_MPL_PP_SUB_DELAY(i,j) \
    /**/

#   define POTHOS_MPL_PP_SUB_DELAY(i,j) \
    POTHOS_PP_CAT(POTHOS_MPL_PP_TUPLE_11_ELEM_##i,POTHOS_MPL_PP_SUB_##j) \
    /**/
#else
#   define POTHOS_MPL_PP_SUB(i,j) \
    POTHOS_MPL_PP_SUB_DELAY(i,j) \
    /**/

#   define POTHOS_MPL_PP_SUB_DELAY(i,j) \
    POTHOS_MPL_PP_TUPLE_11_ELEM_##i POTHOS_MPL_PP_SUB_##j \
    /**/
#endif

#   define POTHOS_MPL_PP_SUB_0 (0,1,2,3,4,5,6,7,8,9,10)
#   define POTHOS_MPL_PP_SUB_1 (0,0,1,2,3,4,5,6,7,8,9)
#   define POTHOS_MPL_PP_SUB_2 (0,0,0,1,2,3,4,5,6,7,8)
#   define POTHOS_MPL_PP_SUB_3 (0,0,0,0,1,2,3,4,5,6,7)
#   define POTHOS_MPL_PP_SUB_4 (0,0,0,0,0,1,2,3,4,5,6)
#   define POTHOS_MPL_PP_SUB_5 (0,0,0,0,0,0,1,2,3,4,5)
#   define POTHOS_MPL_PP_SUB_6 (0,0,0,0,0,0,0,1,2,3,4)
#   define POTHOS_MPL_PP_SUB_7 (0,0,0,0,0,0,0,0,1,2,3)
#   define POTHOS_MPL_PP_SUB_8 (0,0,0,0,0,0,0,0,0,1,2)
#   define POTHOS_MPL_PP_SUB_9 (0,0,0,0,0,0,0,0,0,0,1)
#   define POTHOS_MPL_PP_SUB_10 (0,0,0,0,0,0,0,0,0,0,0)

#else

#   include <Pothos/serialization/impl/preprocessor/arithmetic/sub.hpp>

#   define POTHOS_MPL_PP_SUB(i,j) \
    POTHOS_PP_SUB(i,j) \
    /**/
    
#endif

#endif // BOOST_MPL_AUX_PREPROCESSOR_SUB_HPP_INCLUDED
