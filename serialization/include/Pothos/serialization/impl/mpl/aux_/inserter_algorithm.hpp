
#ifndef POTHOS_MPL_AUX_INSERTER_ALGORITHM_HPP_INCLUDED
#define POTHOS_MPL_AUX_INSERTER_ALGORITHM_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2003-2004
// Copyright David Abrahams 2003-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: inserter_algorithm.hpp 55648 2009-08-18 05:16:53Z agurtovoy $
// $Date: 2009-08-17 22:16:53 -0700 (Mon, 17 Aug 2009) $
// $Revision: 55648 $

#include <Pothos/serialization/impl/mpl/back_inserter.hpp>
#include <Pothos/serialization/impl/mpl/front_inserter.hpp>
#include <Pothos/serialization/impl/mpl/push_back.hpp>
#include <Pothos/serialization/impl/mpl/push_front.hpp>
#include <Pothos/serialization/impl/mpl/back_inserter.hpp>
#include <Pothos/serialization/impl/mpl/front_inserter.hpp>
#include <Pothos/serialization/impl/mpl/clear.hpp>
#include <Pothos/serialization/impl/mpl/eval_if.hpp>
#include <Pothos/serialization/impl/mpl/if.hpp>
#include <Pothos/serialization/impl/mpl/aux_/na.hpp>
#include <Pothos/serialization/impl/mpl/aux_/common_name_wknd.hpp>
#include <Pothos/serialization/impl/mpl/aux_/na_spec.hpp>
#include <Pothos/serialization/impl/mpl/aux_/preprocessor/params.hpp>
#include <Pothos/serialization/impl/mpl/aux_/preprocessor/default_params.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/ctps.hpp>

#include <Pothos/serialization/impl/preprocessor/arithmetic/dec.hpp>

#if !defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

#   define POTHOS_MPL_AUX_INSERTER_ALGORITHM_DEF(arity, name) \
POTHOS_MPL_AUX_COMMON_NAME_WKND(name) \
template< \
      POTHOS_MPL_PP_DEFAULT_PARAMS(arity, typename P, na) \
    > \
struct name \
    : aux::name##_impl<POTHOS_MPL_PP_PARAMS(arity, P)> \
{ \
}; \
\
template< \
      POTHOS_MPL_PP_PARAMS(POTHOS_PP_DEC(arity), typename P) \
    > \
struct name< POTHOS_MPL_PP_PARAMS(POTHOS_PP_DEC(arity), P),na > \
    : if_< has_push_back< typename clear<P1>::type> \
        , aux::name##_impl< \
              POTHOS_MPL_PP_PARAMS(POTHOS_PP_DEC(arity), P) \
            , back_inserter< typename clear<P1>::type > \
            > \
        , aux::reverse_##name##_impl< \
              POTHOS_MPL_PP_PARAMS(POTHOS_PP_DEC(arity), P) \
            , front_inserter< typename clear<P1>::type > \
            > \
        >::type \
{ \
}; \
\
template< \
      POTHOS_MPL_PP_DEFAULT_PARAMS(arity, typename P, na) \
    > \
struct reverse_##name \
    : aux::reverse_##name##_impl<POTHOS_MPL_PP_PARAMS(arity, P)> \
{ \
}; \
\
template< \
      POTHOS_MPL_PP_PARAMS(POTHOS_PP_DEC(arity), typename P) \
    > \
struct reverse_##name< POTHOS_MPL_PP_PARAMS(POTHOS_PP_DEC(arity), P),na > \
    : if_< has_push_back<P1> \
        , aux::reverse_##name##_impl< \
              POTHOS_MPL_PP_PARAMS(POTHOS_PP_DEC(arity), P) \
            , back_inserter< typename clear<P1>::type > \
            > \
        , aux::name##_impl< \
              POTHOS_MPL_PP_PARAMS(POTHOS_PP_DEC(arity), P) \
            , front_inserter< typename clear<P1>::type > \
            > \
        >::type \
{ \
}; \
POTHOS_MPL_AUX_NA_SPEC(arity, name) \
POTHOS_MPL_AUX_NA_SPEC(arity, reverse_##name) \
/**/

#else

#   define POTHOS_MPL_AUX_INSERTER_ALGORITHM_DEF(arity, name) \
POTHOS_MPL_AUX_COMMON_NAME_WKND(name) \
template< \
      POTHOS_MPL_PP_PARAMS(POTHOS_PP_DEC(arity), typename P) \
    > \
struct def_##name##_impl \
    : if_< has_push_back<P1> \
        , aux::name##_impl< \
              POTHOS_MPL_PP_PARAMS(POTHOS_PP_DEC(arity), P) \
            , back_inserter< typename clear<P1>::type > \
            > \
        , aux::reverse_##name##_impl< \
              POTHOS_MPL_PP_PARAMS(POTHOS_PP_DEC(arity), P) \
            , front_inserter< typename clear<P1>::type > \
            > \
        >::type \
{ \
}; \
\
template< \
      POTHOS_MPL_PP_DEFAULT_PARAMS(arity, typename P, na) \
    > \
struct name \
{ \
    typedef typename eval_if< \
          is_na<POTHOS_PP_CAT(P, arity)> \
        , def_##name##_impl<POTHOS_MPL_PP_PARAMS(POTHOS_PP_DEC(arity), P)> \
        , aux::name##_impl<POTHOS_MPL_PP_PARAMS(arity, P)> \
        >::type type; \
}; \
\
template< \
      POTHOS_MPL_PP_PARAMS(POTHOS_PP_DEC(arity), typename P) \
    > \
struct def_reverse_##name##_impl \
    : if_< has_push_back<P1> \
        , aux::reverse_##name##_impl< \
              POTHOS_MPL_PP_PARAMS(POTHOS_PP_DEC(arity), P) \
            , back_inserter< typename clear<P1>::type > \
            > \
        , aux::name##_impl< \
              POTHOS_MPL_PP_PARAMS(POTHOS_PP_DEC(arity), P) \
            , front_inserter< typename clear<P1>::type > \
            > \
        >::type \
{ \
}; \
template< \
      POTHOS_MPL_PP_DEFAULT_PARAMS(arity, typename P, na) \
    > \
struct reverse_##name \
{ \
    typedef typename eval_if< \
          is_na<POTHOS_PP_CAT(P, arity)> \
        , def_reverse_##name##_impl<POTHOS_MPL_PP_PARAMS(POTHOS_PP_DEC(arity), P)> \
        , aux::reverse_##name##_impl<POTHOS_MPL_PP_PARAMS(arity, P)> \
        >::type type; \
}; \
POTHOS_MPL_AUX_NA_SPEC(arity, name) \
POTHOS_MPL_AUX_NA_SPEC(arity, reverse_##name) \
/**/

#endif // BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

#endif // BOOST_MPL_AUX_INSERTER_ALGORITHM_HPP_INCLUDED
