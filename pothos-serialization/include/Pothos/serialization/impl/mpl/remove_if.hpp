
#ifndef POTHOS_MPL_REMOVE_IF_HPP_INCLUDED
#define POTHOS_MPL_REMOVE_IF_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
// Copyright David Abrahams 2003-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: remove_if.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/fold.hpp>
#include <Pothos/serialization/impl/mpl/reverse_fold.hpp>
#include <Pothos/serialization/impl/mpl/eval_if.hpp>
#include <Pothos/serialization/impl/mpl/identity.hpp>
#include <Pothos/serialization/impl/mpl/protect.hpp>
#include <Pothos/serialization/impl/mpl/lambda.hpp>
#include <Pothos/serialization/impl/mpl/apply.hpp>
#include <Pothos/serialization/impl/mpl/aux_/inserter_algorithm.hpp>

namespace Pothos { namespace mpl {

namespace aux {

template< typename Pred, typename InsertOp > struct remove_if_helper
{
    template< typename Sequence, typename U > struct apply
    {
        typedef typename eval_if<
              typename apply1<Pred,U>::type
            , identity<Sequence>
            , apply2<InsertOp,Sequence,U>
            >::type type;
    };
};

template<
      typename Sequence
    , typename Predicate
    , typename Inserter
    >
struct remove_if_impl
    : fold<
          Sequence
        , typename Inserter::state
        , protect< aux::remove_if_helper<
              typename lambda<Predicate>::type
            , typename Inserter::operation
            > >
        >
{
};

template<
      typename Sequence
    , typename Predicate
    , typename Inserter
    >
struct reverse_remove_if_impl
    : reverse_fold<
          Sequence
        , typename Inserter::state
        , protect< aux::remove_if_helper<
              typename lambda<Predicate>::type
            , typename Inserter::operation
            > >
        >
{
};

} // namespace aux

POTHOS_MPL_AUX_INSERTER_ALGORITHM_DEF(3, remove_if)

}}

#endif // BOOST_MPL_REMOVE_IF_HPP_INCLUDED
