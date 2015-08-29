
#ifndef POTHOS_MPL_NEXT_PRIOR_HPP_INCLUDED
#define POTHOS_MPL_NEXT_PRIOR_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: next_prior.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/aux_/common_name_wknd.hpp>
#include <Pothos/serialization/impl/mpl/aux_/na_spec.hpp>
#include <Pothos/serialization/impl/mpl/aux_/lambda_support.hpp>

namespace Pothos { namespace mpl {

POTHOS_MPL_AUX_COMMON_NAME_WKND(next)
POTHOS_MPL_AUX_COMMON_NAME_WKND(prior)

template<
      typename POTHOS_MPL_AUX_NA_PARAM(T)
    >
struct next
{
    typedef typename T::next type;
    POTHOS_MPL_AUX_LAMBDA_SUPPORT(1,next,(T))
};

template<
      typename POTHOS_MPL_AUX_NA_PARAM(T)
    >
struct prior
{
    typedef typename T::prior type;
    POTHOS_MPL_AUX_LAMBDA_SUPPORT(1,prior,(T))
};

POTHOS_MPL_AUX_NA_SPEC(1, next)
POTHOS_MPL_AUX_NA_SPEC(1, prior)

}}

#endif // BOOST_MPL_NEXT_PRIOR_HPP_INCLUDED
