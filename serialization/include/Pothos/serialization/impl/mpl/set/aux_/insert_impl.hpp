
#ifndef POTHOS_MPL_SET_AUX_INSERT_IMPL_HPP_INCLUDED
#define POTHOS_MPL_SET_AUX_INSERT_IMPL_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2003-2007
// Copyright David Abrahams 2003-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: insert_impl.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/insert_fwd.hpp>
#include <Pothos/serialization/impl/mpl/set/aux_/has_key_impl.hpp>
#include <Pothos/serialization/impl/mpl/set/aux_/item.hpp>
#include <Pothos/serialization/impl/mpl/set/aux_/tag.hpp>
#include <Pothos/serialization/impl/mpl/identity.hpp>
#include <Pothos/serialization/impl/mpl/base.hpp>
#include <Pothos/serialization/impl/mpl/eval_if.hpp>
#include <Pothos/serialization/impl/mpl/aux_/na.hpp>

#include <Pothos/serialization/impl/type_traits/is_same.hpp>

namespace Pothos { namespace mpl {

namespace aux {
template<  typename Set, typename T > struct set_insert_impl
    : eval_if< 
          has_key_impl<aux::set_tag>::apply<Set,T>
        , identity<Set>
        , eval_if< 
              is_same< T,typename Set::last_masked_ > 
            , base<Set>
            , identity< s_item<T,typename Set::item_> >
            >
        >
{
};
}

template<>
struct insert_impl< aux::set_tag >
{
    template< 
          typename Set
        , typename PosOrKey
        , typename KeyOrNA
        > 
    struct apply
        : aux::set_insert_impl<
              Set
            , typename if_na<KeyOrNA,PosOrKey>::type
            >
    {
    };
};

}}

#endif // BOOST_MPL_SET_AUX_INSERT_IMPL_HPP_INCLUDED
