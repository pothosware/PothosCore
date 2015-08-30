
#ifndef POTHOS_MPL_MAP_AUX_MAP0_HPP_INCLUDED
#define POTHOS_MPL_MAP_AUX_MAP0_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2003-2004
// Copyright David Abrahams 2003-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: map0.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/long.hpp>
#include <Pothos/serialization/impl/mpl/void.hpp>
#include <Pothos/serialization/impl/mpl/map/aux_/tag.hpp>
#include <Pothos/serialization/impl/mpl/aux_/na.hpp>
#include <Pothos/serialization/impl/mpl/aux_/yes_no.hpp>
#include <Pothos/serialization/impl/mpl/aux_/overload_names.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/operators.hpp>

#include <Pothos/serialization/impl/preprocessor/cat.hpp>

namespace Pothos { namespace mpl {

#if defined(POTHOS_MPL_CFG_USE_OPERATORS_OVERLOADING)

#   define POTHOS_MPL_AUX_MAP0_OVERLOAD(R, f, X, T) \
    friend R POTHOS_PP_CAT(POTHOS_MPL_AUX_OVERLOAD_,f)(X const&, T) \
/**/

#   define POTHOS_MPL_AUX_MAP_OVERLOAD(R, f, X, T) \
    POTHOS_MPL_AUX_MAP0_OVERLOAD(R, f, X, T) \
/**/

#else

#   define POTHOS_MPL_AUX_MAP0_OVERLOAD(R, f, X, T) \
    static R POTHOS_PP_CAT(POTHOS_MPL_AUX_OVERLOAD_,f)(X const&, T) \
/**/

#   define POTHOS_MPL_AUX_MAP_OVERLOAD(R, f, X, T) \
    POTHOS_MPL_AUX_MAP0_OVERLOAD(R, f, X, T); \
    using Base::POTHOS_PP_CAT(POTHOS_MPL_AUX_OVERLOAD_,f) \
/**/

#endif


template< typename Dummy = na > struct map0
{
    typedef map0            type;
    typedef aux::map_tag    tag;
    typedef void_           key_;
    typedef long_<1>        order;
    typedef long_<0>        size;

#if defined(POTHOS_MPL_CFG_TYPEOF_BASED_SEQUENCES)
    POTHOS_MPL_AUX_MAP0_OVERLOAD( aux::type_wrapper<void_>, VALUE_BY_KEY, map0<>, void const volatile* );
    POTHOS_MPL_AUX_MAP0_OVERLOAD( aux::type_wrapper<void_>, ITEM_BY_ORDER, map0<>, long_<1>* );
    POTHOS_MPL_AUX_MAP0_OVERLOAD( aux::no_tag, ORDER_BY_KEY, map0<>, void const volatile* );
#else
    POTHOS_MPL_AUX_MAP0_OVERLOAD( aux::no_tag, ORDER_BY_KEY, map0<>, void const volatile* );
    POTHOS_MPL_AUX_MAP0_OVERLOAD( aux::no_tag, IS_MASKED, map0<>, void const volatile* );
#endif
};

}}

#endif // BOOST_MPL_MAP_AUX_MAP0_HPP_INCLUDED
