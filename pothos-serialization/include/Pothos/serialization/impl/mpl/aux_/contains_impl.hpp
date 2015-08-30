
#ifndef POTHOS_MPL_AUX_CONTAINS_IMPL_HPP_INCLUDED
#define POTHOS_MPL_AUX_CONTAINS_IMPL_HPP_INCLUDED

// Copyright Eric Friedman 2002
// Copyright Aleksey Gurtovoy 2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: contains_impl.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/contains_fwd.hpp>
#include <Pothos/serialization/impl/mpl/begin_end.hpp>
#include <Pothos/serialization/impl/mpl/find.hpp>
#include <Pothos/serialization/impl/mpl/not.hpp>
#include <Pothos/serialization/impl/mpl/aux_/traits_lambda_spec.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/forwarding.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/static_constant.hpp>

#include <Pothos/serialization/impl/type_traits/is_same.hpp>

namespace Pothos { namespace mpl {

template< typename Tag >
struct contains_impl
{
    template< typename Sequence, typename T > struct apply
#if !defined(POTHOS_MPL_CFG_NO_NESTED_FORWARDING)
        : not_< is_same<
              typename find<Sequence,T>::type
            , typename end<Sequence>::type
            > >
    {
#else
    {
        typedef not_< is_same<
              typename find<Sequence,T>::type
            , typename end<Sequence>::type
            > > type;

        POTHOS_STATIC_CONSTANT(bool, value = 
              (not_< is_same<
                  typename find<Sequence,T>::type
                , typename end<Sequence>::type
                > >::value)
            );
#endif
    };
};

POTHOS_MPL_ALGORITM_TRAITS_LAMBDA_SPEC(2,contains_impl)

}}

#endif // BOOST_MPL_AUX_CONTAINS_IMPL_HPP_INCLUDED
