
#ifndef POTHOS_MPL_AUX_ORDER_IMPL_HPP_INCLUDED
#define POTHOS_MPL_AUX_ORDER_IMPL_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2003-2004
// Copyright David Abrahams 2003-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: order_impl.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/order_fwd.hpp>
#include <Pothos/serialization/impl/mpl/if.hpp>
#include <Pothos/serialization/impl/mpl/long.hpp>
#include <Pothos/serialization/impl/mpl/has_key.hpp>
#include <Pothos/serialization/impl/mpl/aux_/overload_names.hpp>
#include <Pothos/serialization/impl/mpl/aux_/static_cast.hpp>
#include <Pothos/serialization/impl/mpl/aux_/type_wrapper.hpp>
#include <Pothos/serialization/impl/mpl/aux_/traits_lambda_spec.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/msvc.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/static_constant.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/workaround.hpp>

namespace Pothos { namespace mpl {

// default implementation; requires 'Seq' to provide corresponding overloads 
// of BOOST_MPL_AUX_OVERLOAD_ORDER_BY_KEY

template< typename Seq, typename Key > struct x_order_impl
#if POTHOS_WORKAROUND(POTHOS_MSVC, POTHOS_TESTED_AT(1400)) \
    || POTHOS_WORKAROUND(__EDG_VERSION__, <= 245)
{
    POTHOS_STATIC_CONSTANT(long, value = 
          sizeof( POTHOS_MPL_AUX_OVERLOAD_CALL_ORDER_BY_KEY(
              Seq
            , POTHOS_MPL_AUX_STATIC_CAST(aux::type_wrapper<Key>*, 0)
            ) )
        );

    typedef long_<value> type;

#else // ISO98 C++
    : long_< 
          sizeof( POTHOS_MPL_AUX_OVERLOAD_CALL_ORDER_BY_KEY(
              Seq
            , POTHOS_MPL_AUX_STATIC_CAST(aux::type_wrapper<Key>*, 0)
            ) )
        >
{
#endif
};

template< typename Tag >
struct order_impl
{
    template< typename Seq, typename Key > struct apply
        : if_<
              typename has_key_impl<Tag>::template apply<Seq,Key>
            , x_order_impl<Seq,Key>
            , void_
            >::type
    {
    };
};

POTHOS_MPL_ALGORITM_TRAITS_LAMBDA_SPEC(2,order_impl)

}}

#endif // BOOST_MPL_AUX_ORDER_IMPL_HPP_INCLUDED
