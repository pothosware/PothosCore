
#ifndef POTHOS_MPL_INSERT_HPP_INCLUDED
#define POTHOS_MPL_INSERT_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: insert.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/insert_fwd.hpp>
#include <Pothos/serialization/impl/mpl/sequence_tag.hpp>
#include <Pothos/serialization/impl/mpl/aux_/insert_impl.hpp>
#include <Pothos/serialization/impl/mpl/aux_/na_spec.hpp>
#include <Pothos/serialization/impl/mpl/aux_/lambda_support.hpp>

namespace Pothos { namespace mpl {

template<
      typename POTHOS_MPL_AUX_NA_PARAM(Sequence)
    , typename POTHOS_MPL_AUX_NA_PARAM(Pos_or_T)
    , typename POTHOS_MPL_AUX_NA_PARAM(T)
    >
struct insert
    : insert_impl< typename sequence_tag<Sequence>::type >
        ::template apply< Sequence,Pos_or_T,T >
{
    POTHOS_MPL_AUX_LAMBDA_SUPPORT(3,insert,(Sequence,Pos_or_T,T))
};

POTHOS_MPL_AUX_NA_SPEC(3, insert)

}}

#endif // BOOST_MPL_INSERT_HPP_INCLUDED
