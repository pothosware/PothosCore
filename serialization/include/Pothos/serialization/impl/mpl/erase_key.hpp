
#ifndef POTHOS_MPL_ERASE_KEY_HPP_INCLUDED
#define POTHOS_MPL_ERASE_KEY_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: erase_key.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/erase_key_fwd.hpp>
#include <Pothos/serialization/impl/mpl/sequence_tag.hpp>
#include <Pothos/serialization/impl/mpl/aux_/erase_key_impl.hpp>
#include <Pothos/serialization/impl/mpl/aux_/na_spec.hpp>
#include <Pothos/serialization/impl/mpl/aux_/lambda_support.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/msvc_typename.hpp>

namespace Pothos { namespace mpl {

template<
      typename POTHOS_MPL_AUX_NA_PARAM(Sequence)
    , typename POTHOS_MPL_AUX_NA_PARAM(Key)
    >
struct erase_key
    : erase_key_impl< typename sequence_tag<Sequence>::type >
        ::template apply< Sequence,Key >
{
    POTHOS_MPL_AUX_LAMBDA_SUPPORT(2,erase_key,(Sequence,Key))
};

POTHOS_MPL_AUX_NA_SPEC(2,erase_key)

}}

#endif // BOOST_MPL_ERASE_KEY_HPP_INCLUDED
