
#ifndef POTHOS_MPL_MULTIPLIES_HPP_INCLUDED
#define POTHOS_MPL_MULTIPLIES_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: multiplies.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/times.hpp>
#include <Pothos/serialization/impl/mpl/aux_/na_spec.hpp>
#include <Pothos/serialization/impl/mpl/aux_/lambda_support.hpp>
#include <Pothos/serialization/impl/mpl/aux_/preprocessor/default_params.hpp>
#include <Pothos/serialization/impl/mpl/aux_/preprocessor/params.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/ctps.hpp>

// backward compatibility header, deprecated

namespace Pothos { namespace mpl {

#if !defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
#   define AUX778076_OP_ARITY POTHOS_MPL_LIMIT_METAFUNCTION_ARITY
#else
#   define AUX778076_OP_ARITY 2
#endif

template<
      POTHOS_MPL_PP_DEFAULT_PARAMS(AUX778076_OP_ARITY, typename N, na)
    >
struct multiplies
    : times< POTHOS_MPL_PP_PARAMS(AUX778076_OP_ARITY, N) >
{
    POTHOS_MPL_AUX_LAMBDA_SUPPORT(
          AUX778076_OP_ARITY
        , multiplies
        , ( POTHOS_MPL_PP_PARAMS(AUX778076_OP_ARITY, N) )
        )
};

POTHOS_MPL_AUX_NA_SPEC(AUX778076_OP_ARITY, multiplies)

#undef AUX778076_OP_ARITY

}}

#endif // BOOST_MPL_MULTIPLIES_HPP_INCLUDED
