
#ifndef POTHOS_MPL_STABLE_PARTITION_HPP_INCLUDED
#define POTHOS_MPL_STABLE_PARTITION_HPP_INCLUDED

// Copyright Eric Friedman 2002-2003
// Copyright Aleksey Gurtovoy 2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: stable_partition.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/fold.hpp>
#include <Pothos/serialization/impl/mpl/reverse_fold.hpp>
#include <Pothos/serialization/impl/mpl/protect.hpp>
#include <Pothos/serialization/impl/mpl/aux_/partition_op.hpp>
#include <Pothos/serialization/impl/mpl/aux_/inserter_algorithm.hpp>
#include <Pothos/serialization/impl/mpl/aux_/na.hpp>

namespace Pothos { namespace mpl {

namespace aux {
 
template <
      typename Sequence
    , typename Pred
    , typename In
    , typename In2
    , typename In1 = typename if_na<In,In2>::type
    >
struct stable_partition_impl
    : fold<
          Sequence
        , pair< typename In1::state, typename In2::state >
        , protect< partition_op< 
              Pred
            , typename In1::operation
            , typename In2::operation
            > >
        >
{
};

template <
      typename Sequence
    , typename Pred
    , typename In
    , typename In2
    , typename In1 = typename if_na<In,In2>::type
    >
struct reverse_stable_partition_impl
    : reverse_fold<
          Sequence
        , pair< typename In1::state, typename In2::state >
        , protect< partition_op< 
              Pred
            , typename In1::operation
            , typename In2::operation
            > >
        >
{
};

} // namespace aux

POTHOS_MPL_AUX_INSERTER_ALGORITHM_DEF(4, stable_partition)

}}

#endif // BOOST_MPL_STABLE_PARTITION_HPP_INCLUDED
