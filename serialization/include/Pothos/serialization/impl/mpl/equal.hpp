
#ifndef POTHOS_MPL_EQUAL_HPP_INCLUDED
#define POTHOS_MPL_EQUAL_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: equal.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/aux_/iter_fold_if_impl.hpp>
#include <Pothos/serialization/impl/mpl/aux_/iter_apply.hpp>
#include <Pothos/serialization/impl/mpl/and.hpp>
#include <Pothos/serialization/impl/mpl/not.hpp>
#include <Pothos/serialization/impl/mpl/begin_end.hpp>
#include <Pothos/serialization/impl/mpl/next.hpp>
#include <Pothos/serialization/impl/mpl/always.hpp>
#include <Pothos/serialization/impl/mpl/bool.hpp>
#include <Pothos/serialization/impl/mpl/lambda.hpp>
#include <Pothos/serialization/impl/mpl/bind.hpp>
#include <Pothos/serialization/impl/mpl/apply.hpp>
#include <Pothos/serialization/impl/mpl/void.hpp>
#include <Pothos/serialization/impl/mpl/aux_/na_spec.hpp>
#include <Pothos/serialization/impl/mpl/aux_/lambda_support.hpp>
#include <Pothos/serialization/impl/mpl/aux_/msvc_eti_base.hpp>

#include <Pothos/serialization/impl/type_traits/is_same.hpp>

namespace Pothos { namespace mpl {

namespace aux {

template<
      typename Predicate
    , typename LastIterator1
    , typename LastIterator2
    >
struct equal_pred
{
    template<
          typename Iterator2
        , typename Iterator1
        >
    struct apply
    {
        typedef typename and_< 
              not_< is_same<Iterator1,LastIterator1> >
            , not_< is_same<Iterator2,LastIterator2> >
            , aux::iter_apply2<Predicate,Iterator1,Iterator2>
            >::type type;
    };
};

template<
      typename Sequence1
    , typename Sequence2
    , typename Predicate
    >
struct equal_impl
{
    typedef typename begin<Sequence1>::type first1_;
    typedef typename begin<Sequence2>::type first2_;
    typedef typename end<Sequence1>::type last1_;
    typedef typename end<Sequence2>::type last2_;

    typedef aux::iter_fold_if_impl<
          first1_
        , first2_
        , next<>
        , protect< aux::equal_pred<Predicate,last1_,last2_> >
        , void_
        , always<false_>
        > fold_;

    typedef typename fold_::iterator iter1_;
    typedef typename fold_::state iter2_;
    typedef and_<
          is_same<iter1_,last1_>
        , is_same<iter2_,last2_>
        > result_;

    typedef typename result_::type type;
};


} // namespace aux


template<
      typename POTHOS_MPL_AUX_NA_PARAM(Sequence1)
    , typename POTHOS_MPL_AUX_NA_PARAM(Sequence2)
    , typename Predicate = is_same<_,_>
    >
struct equal
    : aux::msvc_eti_base< 
          typename aux::equal_impl<Sequence1,Sequence2,Predicate>::type
        >::type
{
    POTHOS_MPL_AUX_LAMBDA_SUPPORT(2,equal,(Sequence1,Sequence2))
};

POTHOS_MPL_AUX_NA_SPEC(2, equal)

}}

#endif // BOOST_MPL_EQUAL_HPP_INCLUDED
