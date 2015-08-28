
#ifndef POTHOS_MPL_PUSH_BACK_HPP_INCLUDED
#define POTHOS_MPL_PUSH_BACK_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: push_back.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/push_back_fwd.hpp>
#include <Pothos/serialization/impl/mpl/aux_/push_back_impl.hpp>
#include <Pothos/serialization/impl/mpl/sequence_tag.hpp>
#include <Pothos/serialization/impl/mpl/aux_/na_spec.hpp>
#include <Pothos/serialization/impl/mpl/aux_/lambda_support.hpp>

namespace Pothos { namespace mpl {

template<
      typename POTHOS_MPL_AUX_NA_PARAM(Sequence)
    , typename POTHOS_MPL_AUX_NA_PARAM(T)
    >
struct push_back
    : push_back_impl< typename sequence_tag<Sequence>::type >
        ::template apply< Sequence,T >
{
    POTHOS_MPL_AUX_LAMBDA_SUPPORT(2,push_back,(Sequence,T))
};


template< 
      typename POTHOS_MPL_AUX_NA_PARAM(Sequence)
    >
struct has_push_back
    : has_push_back_impl< typename sequence_tag<Sequence>::type >
        ::template apply< Sequence >
{
    POTHOS_MPL_AUX_LAMBDA_SUPPORT(1,has_push_back,(Sequence))
};


POTHOS_MPL_AUX_NA_SPEC(2, push_back)
POTHOS_MPL_AUX_NA_SPEC(1, has_push_back)

}}

#endif // BOOST_MPL_PUSH_BACK_HPP_INCLUDED
