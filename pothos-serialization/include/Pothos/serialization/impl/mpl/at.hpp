
#ifndef POTHOS_MPL_AT_HPP_INCLUDED
#define POTHOS_MPL_AT_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: at.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/at_fwd.hpp>
#include <Pothos/serialization/impl/mpl/aux_/at_impl.hpp>
#include <Pothos/serialization/impl/mpl/long.hpp>
#include <Pothos/serialization/impl/mpl/sequence_tag.hpp>
#include <Pothos/serialization/impl/mpl/aux_/na_spec.hpp>
#include <Pothos/serialization/impl/mpl/aux_/lambda_support.hpp>
#include <Pothos/serialization/impl/mpl/aux_/nttp_decl.hpp>

namespace Pothos { namespace mpl {

template<
      typename POTHOS_MPL_AUX_NA_PARAM(Sequence)
    , typename POTHOS_MPL_AUX_NA_PARAM(N)
    >
struct at
    : at_impl< typename sequence_tag<Sequence>::type >
        ::template apply< Sequence,N >
{
    POTHOS_MPL_AUX_LAMBDA_SUPPORT(2,at,(Sequence,N))
};

template<
      typename Sequence
    , POTHOS_MPL_AUX_NTTP_DECL(long, N)
    >
struct at_c
    : at_impl< typename sequence_tag<Sequence>::type >
        ::template apply< Sequence,mpl::long_<N> >
{
};

POTHOS_MPL_AUX_NA_SPEC(2, at)

}}

#endif // BOOST_MPL_AT_HPP_INCLUDED
