
#if !defined(POTHOS_PP_IS_ITERATING)

///// header body

#ifndef POTHOS_MPL_BIND_FWD_HPP_INCLUDED
#define POTHOS_MPL_BIND_FWD_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: bind_fwd.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#if !defined(POTHOS_MPL_PREPROCESSING_MODE)
#   include <Pothos/serialization/impl/mpl/aux_/na.hpp>
#endif

#include <Pothos/serialization/impl/mpl/aux_/config/bind.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/use_preprocessed.hpp>

#if !defined(POTHOS_MPL_CFG_NO_PREPROCESSED_HEADERS) \
    && !defined(POTHOS_MPL_PREPROCESSING_MODE)

#   define POTHOS_MPL_PREPROCESSED_HEADER bind_fwd.hpp
#   include <Pothos/serialization/impl/mpl/aux_/include_preprocessed.hpp>

#else

#   include <Pothos/serialization/impl/mpl/limits/arity.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/params.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/default_params.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/dmc_ambiguous_ctps.hpp>

#   include <Pothos/serialization/impl/preprocessor/comma_if.hpp>
#   include <Pothos/serialization/impl/preprocessor/iterate.hpp>
#   include <Pothos/serialization/impl/preprocessor/cat.hpp>

namespace Pothos { namespace mpl {

// local macros, #undef-ined at the end of the header

#   if defined(POTHOS_MPL_CFG_DMC_AMBIGUOUS_CTPS)
#       define AUX778076_DMC_PARAM() , int dummy_ = 0
#   else
#       define AUX778076_DMC_PARAM()
#   endif

#   define AUX778076_BIND_DEFAULT_PARAMS(param, value) \
    POTHOS_MPL_PP_DEFAULT_PARAMS( \
          POTHOS_MPL_LIMIT_METAFUNCTION_ARITY \
        , param \
        , value \
        ) \
    AUX778076_DMC_PARAM() \
    /**/

#   define AUX778076_BIND_N_PARAMS(n, param) \
    POTHOS_PP_COMMA_IF(n) POTHOS_MPL_PP_PARAMS(n, param) \
    AUX778076_DMC_PARAM() \
    /**/

#if !defined(POTHOS_MPL_CFG_NO_BIND_TEMPLATE)
template<
      typename F, AUX778076_BIND_DEFAULT_PARAMS(typename T, na)
    >
struct bind;
#endif

#define POTHOS_PP_ITERATION_PARAMS_1 \
    (3,(0, POTHOS_MPL_LIMIT_METAFUNCTION_ARITY, <boost/mpl/bind_fwd.hpp>))
#include POTHOS_PP_ITERATE()

#   undef AUX778076_BIND_N_PARAMS
#   undef AUX778076_BIND_DEFAULT_PARAMS
#   undef AUX778076_DMC_PARAM
}}

#endif // BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#endif // BOOST_MPL_BIND_FWD_HPP_INCLUDED

///// iteration

#else
#define i_ POTHOS_PP_FRAME_ITERATION(1)

template<
      typename F AUX778076_BIND_N_PARAMS(i_, typename T)
    >
struct POTHOS_PP_CAT(bind,i_);

#undef i_
#endif // BOOST_PP_IS_ITERATING
