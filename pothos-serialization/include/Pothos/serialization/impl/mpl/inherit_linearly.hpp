
#ifndef POTHOS_MPL_INHERIT_FRONT_TO_BACK_HPP_INCLUDED
#define POTHOS_MPL_INHERIT_FRONT_TO_BACK_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2001-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: inherit_linearly.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/fold.hpp>
#include <Pothos/serialization/impl/mpl/empty_base.hpp>
#include <Pothos/serialization/impl/mpl/aux_/na_spec.hpp>
#include <Pothos/serialization/impl/mpl/aux_/lambda_support.hpp>

namespace Pothos { namespace mpl {

template<
      typename POTHOS_MPL_AUX_NA_PARAM(Types_)
    , typename POTHOS_MPL_AUX_NA_PARAM(Node_)
    , typename Root_ = empty_base
    >
struct inherit_linearly
    : fold<Types_,Root_,Node_>
{
    POTHOS_MPL_AUX_LAMBDA_SUPPORT(3,inherit_linearly,(Types_,Node_,Root_))
};

POTHOS_MPL_AUX_NA_SPEC(2, inherit_linearly)

}}

#endif // BOOST_MPL_INHERIT_FRONT_TO_BACK_HPP_INCLUDED
