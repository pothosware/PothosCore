
#ifndef POTHOS_MPL_AUX_RANGE_C_ITERATOR_HPP_INCLUDED
#define POTHOS_MPL_AUX_RANGE_C_ITERATOR_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: iterator.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/iterator_tags.hpp>
#include <Pothos/serialization/impl/mpl/advance_fwd.hpp>
#include <Pothos/serialization/impl/mpl/distance_fwd.hpp>
#include <Pothos/serialization/impl/mpl/next_prior.hpp>
#include <Pothos/serialization/impl/mpl/deref.hpp>
#include <Pothos/serialization/impl/mpl/plus.hpp>
#include <Pothos/serialization/impl/mpl/minus.hpp>
#include <Pothos/serialization/impl/mpl/aux_/value_wknd.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/ctps.hpp>

namespace Pothos { namespace mpl {

// theoretically will work on any discrete numeric type
template< typename N > struct r_iter
{
    typedef aux::r_iter_tag tag;
    typedef random_access_iterator_tag category;
    typedef N type;

#if defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
    typedef r_iter< typename mpl::next<N>::type > next;
    typedef r_iter< typename mpl::prior<N>::type > prior;
#endif
};

#if !defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

template<
      typename N
    >
struct next< r_iter<N> >
{
    typedef r_iter< typename mpl::next<N>::type > type;
};

template<
      typename N
    >
struct prior< r_iter<N> >
{
    typedef r_iter< typename mpl::prior<N>::type > type;
};

#endif


template<> struct advance_impl<aux::r_iter_tag>
{
    template< typename Iter, typename Dist > struct apply
    {
        typedef typename deref<Iter>::type n_;
#if POTHOS_WORKAROUND(POTHOS_MSVC, <= 1300)
        typedef typename plus_impl<integral_c_tag,integral_c_tag>
            ::template apply<n_,Dist>::type m_;
#else
        typedef typename plus<n_,Dist>::type m_;
#endif
        // agurt, 10/nov/04: to be generic, the code have to do something along
        // the lines below...
        //
        // typedef typename apply_wrap1<
        //       numeric_cast< typename m_::tag, typename n_::tag >
        //     , m_
        //     >::type result_;
        //
        // ... meanwhile:
        
        typedef integral_c< 
              typename aux::value_type_wknd<n_>::type
            , POTHOS_MPL_AUX_VALUE_WKND(m_)::value 
            > result_;
        
        typedef r_iter<result_> type;
    };
};

template<> struct distance_impl<aux::r_iter_tag>
{
    template< typename Iter1, typename Iter2 > struct apply
        : minus<
              typename Iter2::type
            , typename Iter1::type
            >
    {
    };
};

}}

#endif // BOOST_MPL_AUX_RANGE_C_ITERATOR_HPP_INCLUDED
