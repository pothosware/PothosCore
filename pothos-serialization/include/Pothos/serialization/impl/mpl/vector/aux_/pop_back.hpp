
#ifndef POTHOS_MPL_VECTOR_AUX_POP_BACK_HPP_INCLUDED
#define POTHOS_MPL_VECTOR_AUX_POP_BACK_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: pop_back.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/pop_back_fwd.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/typeof.hpp>

#if defined(POTHOS_MPL_CFG_TYPEOF_BASED_SEQUENCES)

#   include <Pothos/serialization/impl/mpl/vector/aux_/item.hpp>
#   include <Pothos/serialization/impl/mpl/vector/aux_/tag.hpp>

namespace Pothos { namespace mpl {

template<>
struct pop_back_impl< aux::vector_tag >
{
    template< typename Vector > struct apply
    {
        typedef v_mask<Vector,0> type;
    };
};

}}

#endif // BOOST_MPL_CFG_TYPEOF_BASED_SEQUENCES

#endif // BOOST_MPL_VECTOR_AUX_POP_BACK_HPP_INCLUDED
