
#ifndef POTHOS_MPL_AUX_OVERLOAD_NAMES_HPP_INCLUDED
#define POTHOS_MPL_AUX_OVERLOAD_NAMES_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: overload_names.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/aux_/ptr_to_ref.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/operators.hpp>

#if defined(POTHOS_MPL_CFG_USE_OPERATORS_OVERLOADING)

#   include <Pothos/serialization/impl/mpl/aux_/static_cast.hpp>

#   define POTHOS_MPL_AUX_OVERLOAD_VALUE_BY_KEY  operator/
#   define POTHOS_MPL_AUX_OVERLOAD_ITEM_BY_ORDER operator|
#   define POTHOS_MPL_AUX_OVERLOAD_ORDER_BY_KEY  operator||
#   define POTHOS_MPL_AUX_OVERLOAD_IS_MASKED     operator%

#   define POTHOS_MPL_AUX_OVERLOAD_CALL_VALUE_BY_KEY(T, x)   POTHOS_MPL_AUX_PTR_TO_REF(T) / x
#   define POTHOS_MPL_AUX_OVERLOAD_CALL_ITEM_BY_ORDER(T, x)  POTHOS_MPL_AUX_PTR_TO_REF(T) | x
#   define POTHOS_MPL_AUX_OVERLOAD_CALL_ORDER_BY_KEY(T, x)   POTHOS_MPL_AUX_PTR_TO_REF(T) || x
#   define POTHOS_MPL_AUX_OVERLOAD_CALL_IS_MASKED(T, x)      POTHOS_MPL_AUX_PTR_TO_REF(T) % x

#else

#   define POTHOS_MPL_AUX_OVERLOAD_VALUE_BY_KEY  value_by_key_
#   define POTHOS_MPL_AUX_OVERLOAD_ITEM_BY_ORDER item_by_order_
#   define POTHOS_MPL_AUX_OVERLOAD_ORDER_BY_KEY  order_by_key_
#   define POTHOS_MPL_AUX_OVERLOAD_IS_MASKED     is_masked_

#   define POTHOS_MPL_AUX_OVERLOAD_CALL_VALUE_BY_KEY(T, x)   T::POTHOS_MPL_AUX_OVERLOAD_VALUE_BY_KEY( POTHOS_MPL_AUX_PTR_TO_REF(T), x )
#   define POTHOS_MPL_AUX_OVERLOAD_CALL_ITEM_BY_ORDER(T, x)  T::POTHOS_MPL_AUX_OVERLOAD_ITEM_BY_ORDER( POTHOS_MPL_AUX_PTR_TO_REF(T), x )
#   define POTHOS_MPL_AUX_OVERLOAD_CALL_ORDER_BY_KEY(T, x)   T::POTHOS_MPL_AUX_OVERLOAD_ORDER_BY_KEY( POTHOS_MPL_AUX_PTR_TO_REF(T), x )
#   define POTHOS_MPL_AUX_OVERLOAD_CALL_IS_MASKED(T, x)      T::POTHOS_MPL_AUX_OVERLOAD_IS_MASKED( POTHOS_MPL_AUX_PTR_TO_REF(T), x )

#endif

#endif // BOOST_MPL_AUX_OVERLOAD_NAMES_HPP_INCLUDED
