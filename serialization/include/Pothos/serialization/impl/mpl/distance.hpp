
#ifndef POTHOS_MPL_DISTANCE_HPP_INCLUDED
#define POTHOS_MPL_DISTANCE_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: distance.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/distance_fwd.hpp>
#include <Pothos/serialization/impl/mpl/iter_fold.hpp>
#include <Pothos/serialization/impl/mpl/iterator_range.hpp>
#include <Pothos/serialization/impl/mpl/long.hpp>
#include <Pothos/serialization/impl/mpl/next.hpp>
#include <Pothos/serialization/impl/mpl/tag.hpp>
#include <Pothos/serialization/impl/mpl/apply_wrap.hpp>
#include <Pothos/serialization/impl/mpl/aux_/msvc_eti_base.hpp>
#include <Pothos/serialization/impl/mpl/aux_/value_wknd.hpp>
#include <Pothos/serialization/impl/mpl/aux_/na_spec.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/forwarding.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/static_constant.hpp>


namespace Pothos { namespace mpl {

// default implementation for forward/bidirectional iterators
template< typename Tag > struct distance_impl
{
    template< typename First, typename Last > struct apply
#if !defined(POTHOS_MPL_CFG_NO_NESTED_FORWARDING)
        : aux::msvc_eti_base< typename iter_fold<
              iterator_range<First,Last>
            , mpl::long_<0>
            , next<>
            >::type >
    {
#else
    {
        typedef typename iter_fold<
              iterator_range<First,Last>
            , mpl::long_<0>
            , next<>
            >::type type;
        
        POTHOS_STATIC_CONSTANT(long, value =
              (iter_fold<
                  iterator_range<First,Last>
                , mpl::long_<0>
                , next<>
                >::type::value)
            );
#endif
    };
};

template<
      typename POTHOS_MPL_AUX_NA_PARAM(First)
    , typename POTHOS_MPL_AUX_NA_PARAM(Last)
    >
struct distance
    : distance_impl< typename tag<First>::type >
        ::template apply<First, Last>
{
    POTHOS_MPL_AUX_LAMBDA_SUPPORT(2, distance, (First, Last))
};

POTHOS_MPL_AUX_NA_SPEC(2, distance)

}}

#endif // BOOST_MPL_DISTANCE_HPP_INCLUDED
