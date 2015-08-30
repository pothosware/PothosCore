
#ifndef POTHOS_MPL_SET_AUX_HAS_KEY_IMPL_HPP_INCLUDED
#define POTHOS_MPL_SET_AUX_HAS_KEY_IMPL_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2003-2004
// Copyright David Abrahams 2003-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: has_key_impl.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/set/aux_/tag.hpp>
#include <Pothos/serialization/impl/mpl/has_key_fwd.hpp>
#include <Pothos/serialization/impl/mpl/bool.hpp>
#include <Pothos/serialization/impl/mpl/aux_/overload_names.hpp>
#include <Pothos/serialization/impl/mpl/aux_/static_cast.hpp>
#include <Pothos/serialization/impl/mpl/aux_/yes_no.hpp>
#include <Pothos/serialization/impl/mpl/aux_/type_wrapper.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/workaround.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/static_constant.hpp>

namespace Pothos { namespace mpl {

template<>
struct has_key_impl< aux::set_tag >
{
    template< typename Set, typename T > struct apply
#if POTHOS_WORKAROUND(POTHOS_MSVC, POTHOS_TESTED_AT(1400)) \
    || POTHOS_WORKAROUND(__EDG_VERSION__, <= 245)
    {
        POTHOS_STATIC_CONSTANT(bool, value = 
              ( sizeof( POTHOS_MPL_AUX_OVERLOAD_CALL_IS_MASKED(
                    Set
                  , POTHOS_MPL_AUX_STATIC_CAST(aux::type_wrapper<T>*, 0)
                  ) ) == sizeof(aux::no_tag) )
            );

        typedef bool_<value> type;

#else // ISO98 C++
        : bool_< 
              ( sizeof( POTHOS_MPL_AUX_OVERLOAD_CALL_IS_MASKED(
                    Set
                  , POTHOS_MPL_AUX_STATIC_CAST(aux::type_wrapper<T>*, 0)
                  ) ) == sizeof(aux::no_tag) )
            >
    {
#endif
    };
};

}}

#endif // BOOST_MPL_SET_AUX_HAS_KEY_IMPL_HPP_INCLUDED
