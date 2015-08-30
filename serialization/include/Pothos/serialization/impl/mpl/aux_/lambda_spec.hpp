
#ifndef POTHOS_MPL_AUX_LAMBDA_SPEC_HPP_INCLUDED
#define POTHOS_MPL_AUX_LAMBDA_SPEC_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2001-2007
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: lambda_spec.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/void.hpp>
#include <Pothos/serialization/impl/mpl/lambda_fwd.hpp>
#include <Pothos/serialization/impl/mpl/int_fwd.hpp>
#include <Pothos/serialization/impl/mpl/aux_/preprocessor/params.hpp>
#include <Pothos/serialization/impl/mpl/aux_/lambda_arity_param.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/lambda.hpp>

#if !defined(POTHOS_MPL_CFG_NO_FULL_LAMBDA_SUPPORT)

#   define POTHOS_MPL_AUX_PASS_THROUGH_LAMBDA_SPEC(i, name) \
template< \
      POTHOS_MPL_PP_PARAMS(i, typename T) \
    , typename Tag \
    > \
struct lambda< \
      name< POTHOS_MPL_PP_PARAMS(i, T) > \
    , Tag \
    POTHOS_MPL_AUX_LAMBDA_ARITY_PARAM(int_<i>) \
    > \
{ \
    typedef false_ is_le; \
    typedef name< POTHOS_MPL_PP_PARAMS(i, T) > result_; \
    typedef result_ type; \
}; \
/**/

#else

#   define POTHOS_MPL_AUX_PASS_THROUGH_LAMBDA_SPEC(i, name) /**/

#endif

#endif // BOOST_MPL_AUX_LAMBDA_SPEC_HPP_INCLUDED
