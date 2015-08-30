
#ifndef POTHOS_MPL_MAP_AUX_ITERATOR_HPP_INCLUDED
#define POTHOS_MPL_MAP_AUX_ITERATOR_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2003-2004
// Copyright David Abrahams 2003-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: iterator.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/map/aux_/map0.hpp>
#include <Pothos/serialization/impl/mpl/map/aux_/at_impl.hpp>
#include <Pothos/serialization/impl/mpl/map/aux_/tag.hpp>
#include <Pothos/serialization/impl/mpl/iterator_tags.hpp>
#include <Pothos/serialization/impl/mpl/if.hpp>
#include <Pothos/serialization/impl/mpl/next.hpp>
#include <Pothos/serialization/impl/mpl/deref.hpp>
#include <Pothos/serialization/impl/mpl/long.hpp>
#include <Pothos/serialization/impl/mpl/void.hpp>
#include <Pothos/serialization/impl/mpl/aux_/nttp_decl.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/ctps.hpp>

namespace Pothos { namespace mpl {

#if !defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

template< 
      typename Map
    , long order
    , long max_order
    >
struct next_order
    : if_< 
          is_void_< typename item_by_order<Map,order>::type >
        , next_order<Map,(order+1),max_order>
        , long_<order>
        >::type
{
};

template< 
      typename Map
    , long max_order
    >
struct next_order<Map,max_order,max_order>
    : long_<max_order>
{
};


template< typename Map, long order, long max_order >
struct m_iter
{
    typedef forward_iterator_tag category;
    typedef typename item_by_order<Map,order>::type type;
};

template< typename Map, long max_order >
struct m_iter<Map,max_order,max_order>
{
    typedef forward_iterator_tag category;
};


template< typename Map, long order, long max_order > 
struct next< m_iter<Map,order,max_order> >
{
    typedef m_iter<
          Map
        , next_order<Map,order+1,max_order>::value
        , max_order
        > type;
};

template< typename Map, long max_order > 
struct next< m_iter<Map,max_order,max_order> >
{
};

#else

template< 
      typename Map
    , POTHOS_MPL_AUX_NTTP_DECL(long, order)
    , POTHOS_MPL_AUX_NTTP_DECL(long, max_order)
    >
struct next_order;

template< 
      typename Map
    , POTHOS_MPL_AUX_NTTP_DECL(long, order)
    , POTHOS_MPL_AUX_NTTP_DECL(long, max_order)
    >
struct next_order_impl
    : if_< 
          is_void_< typename item_by_order<Map,order>::type >
        , next_order<Map,(order+1),max_order>
        , long_<order>
        >::type
    {
    };

template< 
      typename Map
    , POTHOS_MPL_AUX_NTTP_DECL(long, order)
    , POTHOS_MPL_AUX_NTTP_DECL(long, max_order)
    >
struct next_order
    : if_c<
          (order != max_order)
        , next_order_impl<Map,order,max_order>
        , long_<order>
        >::type
{
};


template<
      typename Map
    , POTHOS_MPL_AUX_NTTP_DECL(long, order)
    , POTHOS_MPL_AUX_NTTP_DECL(long, max_order)
    >
struct m_iter;

struct m_iter_empty_base {};

template< 
      typename Map
    , POTHOS_MPL_AUX_NTTP_DECL(long, order)
    , POTHOS_MPL_AUX_NTTP_DECL(long, max_order)
    >
struct m_iter_base
{
    typedef typename item_by_order<Map,order>::type type;
    
    typedef m_iter<
          Map
        , next_order<Map,order+1,max_order>::value
        , max_order
        > next;
};

template<
      typename Map
    , POTHOS_MPL_AUX_NTTP_DECL(long, order)
    , POTHOS_MPL_AUX_NTTP_DECL(long, max_order)
    >
struct m_iter
  : if_c<
          (order == max_order)
        , m_iter_empty_base
        , m_iter_base<Map,order,max_order>
        >::type
{
    typedef forward_iterator_tag category;
};

#endif // BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

}}

#endif // BOOST_MPL_MAP_AUX_ITERATOR_HPP_INCLUDED
