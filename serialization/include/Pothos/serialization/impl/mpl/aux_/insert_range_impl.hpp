
#ifndef POTHOS_MPL_AUX_INSERT_RANGE_IMPL_HPP_INCLUDED
#define POTHOS_MPL_AUX_INSERT_RANGE_IMPL_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: insert_range_impl.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/copy.hpp>
#include <Pothos/serialization/impl/mpl/clear.hpp>
#include <Pothos/serialization/impl/mpl/front_inserter.hpp>
#include <Pothos/serialization/impl/mpl/joint_view.hpp>
#include <Pothos/serialization/impl/mpl/iterator_range.hpp>
#include <Pothos/serialization/impl/mpl/aux_/na_spec.hpp>
#include <Pothos/serialization/impl/mpl/aux_/iter_push_front.hpp>
#include <Pothos/serialization/impl/mpl/aux_/traits_lambda_spec.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/forwarding.hpp>

#include <Pothos/serialization/impl/type_traits/same_traits.hpp>

namespace Pothos { namespace mpl {

// default implementation; conrete sequences might override it by 
// specializing either the 'insert_range_impl' or the primary 
// 'insert_range' template


template< typename Tag >
struct insert_range_impl
{
    template<
          typename Sequence
        , typename Pos
        , typename Range
        >
    struct apply
#if !defined(POTHOS_MPL_CFG_NO_NESTED_FORWARDING)
        : reverse_copy<
              joint_view< 
                  iterator_range<typename begin<Sequence>::type,Pos>
                , joint_view< 
                      Range
                    , iterator_range<Pos,typename end<Sequence>::type>
                    >
                >
            , front_inserter< typename clear<Sequence>::type >
            >
    {
#else
    {
        typedef typename reverse_copy<
              joint_view< 
                  iterator_range<typename begin<Sequence>::type,Pos>
                , joint_view< 
                      Range
                    , iterator_range<Pos,typename end<Sequence>::type>
                    >
                >
            , front_inserter< typename clear<Sequence>::type >
            >::type type;
#endif
    };
};

POTHOS_MPL_ALGORITM_TRAITS_LAMBDA_SPEC(3,insert_range_impl)

}}

#endif // BOOST_MPL_AUX_INSERT_RANGE_IMPL_HPP_INCLUDED
