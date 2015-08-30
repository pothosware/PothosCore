
#ifndef POTHOS_MPL_MAP_AUX_HAS_KEY_IMPL_HPP_INCLUDED
#define POTHOS_MPL_MAP_AUX_HAS_KEY_IMPL_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2003-2004
// Copyright David Abrahams 2003-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: has_key_impl.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/has_key_fwd.hpp>
#include <Pothos/serialization/impl/mpl/map/aux_/tag.hpp>
#include <Pothos/serialization/impl/mpl/map/aux_/at_impl.hpp>
#include <Pothos/serialization/impl/mpl/void.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/typeof.hpp>

namespace Pothos { namespace mpl {

template<>
struct has_key_impl< aux::map_tag >
{
    template< typename Map, typename Key > struct apply
#if defined(POTHOS_MPL_CFG_TYPEOF_BASED_SEQUENCES)
        : is_not_void_< 
              typename at_impl<aux::map_tag>
                ::apply<Map,Key>::type
            >
#else
        : bool_< ( x_order_impl<Map,Key>::value > 1 ) >
#endif
    {
    };
};

}}

#endif // BOOST_MPL_MAP_AUX_HAS_KEY_IMPL_HPP_INCLUDED
