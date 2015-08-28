
#ifndef POTHOS_MPL_MAP_AUX_ERASE_KEY_IMPL_HPP_INCLUDED
#define POTHOS_MPL_MAP_AUX_ERASE_KEY_IMPL_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2003-2004
// Copyright David Abrahams 2003-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: erase_key_impl.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/erase_key_fwd.hpp>
#include <Pothos/serialization/impl/mpl/map/aux_/has_key_impl.hpp>
#include <Pothos/serialization/impl/mpl/map/aux_/item.hpp>
#include <Pothos/serialization/impl/mpl/map/aux_/tag.hpp>
#include <Pothos/serialization/impl/mpl/identity.hpp>
#include <Pothos/serialization/impl/mpl/base.hpp>
#include <Pothos/serialization/impl/mpl/eval_if.hpp>

#include <Pothos/serialization/impl/type_traits/is_same.hpp>

namespace Pothos { namespace mpl {

template<>
struct erase_key_impl< aux::map_tag >
{
    template< 
          typename Map
        , typename Key
        > 
    struct apply
        : eval_if< 
              has_key_impl<aux::map_tag>::apply<Map,Key>
            , eval_if< 
                  is_same< Key,typename Map::key_ > 
                , base<Map>
                , identity< m_mask<Key,Map> >
                >
            , identity<Map>
            >
    {
    };
};

}}

#endif // BOOST_MPL_MAP_AUX_ERASE_KEY_IMPL_HPP_INCLUDED
