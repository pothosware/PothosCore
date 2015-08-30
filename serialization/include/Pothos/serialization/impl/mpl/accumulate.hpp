
#ifndef POTHOS_MPL_ACCUMULATE_HPP_INCLUDED
#define POTHOS_MPL_ACCUMULATE_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2001-2004
// Copyright David Abrahams 2001-2002
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: accumulate.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/fold.hpp>
#include <Pothos/serialization/impl/mpl/aux_/na_spec.hpp>
#include <Pothos/serialization/impl/mpl/aux_/lambda_support.hpp>

namespace Pothos { namespace mpl {

template<
      typename POTHOS_MPL_AUX_NA_PARAM(Sequence)
    , typename POTHOS_MPL_AUX_NA_PARAM(State)
    , typename POTHOS_MPL_AUX_NA_PARAM(ForwardOp)
    >
struct accumulate
    : fold<Sequence,State,ForwardOp>
{
    POTHOS_MPL_AUX_LAMBDA_SUPPORT(3,accumulate,(Sequence,State,ForwardOp))
};

POTHOS_MPL_AUX_NA_SPEC(3, accumulate)

}}

#endif // BOOST_MPL_ACCUMULATE_HPP_INCLUDED
