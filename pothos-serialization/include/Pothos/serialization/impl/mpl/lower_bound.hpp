
#ifndef POTHOS_MPL_LOWER_BOUND_HPP_INCLUDED
#define POTHOS_MPL_LOWER_BOUND_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2001-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: lower_bound.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/less.hpp>
#include <Pothos/serialization/impl/mpl/lambda.hpp>
#include <Pothos/serialization/impl/mpl/aux_/na_spec.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/workaround.hpp>

#if POTHOS_WORKAROUND(__BORLANDC__, POTHOS_TESTED_AT(0x610))
#   define POTHOS_MPL_CFG_STRIPPED_DOWN_LOWER_BOUND_IMPL
#endif

#if !defined(POTHOS_MPL_CFG_STRIPPED_DOWN_LOWER_BOUND_IMPL)
#   include <Pothos/serialization/impl/mpl/minus.hpp>
#   include <Pothos/serialization/impl/mpl/divides.hpp>
#   include <Pothos/serialization/impl/mpl/size.hpp>
#   include <Pothos/serialization/impl/mpl/advance.hpp>
#   include <Pothos/serialization/impl/mpl/begin_end.hpp>
#   include <Pothos/serialization/impl/mpl/long.hpp>
#   include <Pothos/serialization/impl/mpl/eval_if.hpp>
#   include <Pothos/serialization/impl/mpl/prior.hpp>
#   include <Pothos/serialization/impl/mpl/deref.hpp>
#   include <Pothos/serialization/impl/mpl/apply.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/value_wknd.hpp>
#else
#   include <Pothos/serialization/impl/mpl/not.hpp>
#   include <Pothos/serialization/impl/mpl/find.hpp>
#   include <Pothos/serialization/impl/mpl/bind.hpp>
#endif

#include <Pothos/serialization/impl/config.hpp>

namespace Pothos { namespace mpl {

#if defined(POTHOS_MPL_CFG_STRIPPED_DOWN_LOWER_BOUND_IMPL)

// agurt 23/oct/02: has a wrong complexity etc., but at least it works
// feel free to contribute a better implementation!
template<
      typename POTHOS_MPL_AUX_NA_PARAM(Sequence)
    , typename POTHOS_MPL_AUX_NA_PARAM(T)
    , typename Predicate = less<>
    , typename pred_ = typename lambda<Predicate>::type
    >
struct lower_bound
    : find_if< Sequence, bind1< not_<>, bind2<pred_,_,T> > >
{
};

#else

namespace aux {

template<
      typename Distance
    , typename Predicate
    , typename T
    , typename DeferredIterator
    >
struct lower_bound_step_impl;

template< 
      typename Distance
    , typename Predicate
    , typename T
    , typename DeferredIterator
    >
struct lower_bound_step
{
    typedef typename eval_if<
          Distance
        , lower_bound_step_impl<Distance,Predicate,T,DeferredIterator>
        , DeferredIterator
        >::type type;
};
    
template<
      typename Distance
    , typename Predicate
    , typename T
    , typename DeferredIterator
    >
struct lower_bound_step_impl
{
    typedef typename divides< Distance, long_<2> >::type offset_;
    typedef typename DeferredIterator::type iter_;
    typedef typename advance< iter_,offset_ >::type middle_;
    typedef typename apply2<
              Predicate
            , typename deref<middle_>::type
            , T
            >::type cond_;

    typedef typename prior< minus< Distance, offset_> >::type step_;
    typedef lower_bound_step< offset_,Predicate,T,DeferredIterator > step_forward_;
    typedef lower_bound_step< step_,Predicate,T,next<middle_> > step_backward_;
    typedef typename eval_if<
          cond_
        , step_backward_
        , step_forward_
        >::type type;
};


} // namespace aux

template<
      typename POTHOS_MPL_AUX_NA_PARAM(Sequence)
    , typename POTHOS_MPL_AUX_NA_PARAM(T)
    , typename Predicate = less<>
    >
struct lower_bound
{
 private:
    typedef typename lambda<Predicate>::type pred_;
    typedef typename size<Sequence>::type size_;

 public:
    typedef typename aux::lower_bound_step<
        size_,pred_,T,begin<Sequence>
        >::type type;
};

#endif // BOOST_MPL_CFG_STRIPPED_DOWN_LOWER_BOUND_IMPL

POTHOS_MPL_AUX_NA_SPEC(2, lower_bound)

}}

#endif // BOOST_MPL_LOWER_BOUND_HPP_INCLUDED
