
#ifndef POTHOS_MPL_AUX_HAS_REBIND_HPP_INCLUDED
#define POTHOS_MPL_AUX_HAS_REBIND_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2002-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: has_rebind.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/aux_/config/msvc.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/intel.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/workaround.hpp>

#if POTHOS_WORKAROUND(__EDG_VERSION__, <= 244) && !defined(POTHOS_INTEL_CXX_VERSION)
#   include <Pothos/serialization/impl/mpl/has_xxx.hpp>
#elif POTHOS_WORKAROUND(POTHOS_MSVC, < 1300)
#   include <Pothos/serialization/impl/mpl/has_xxx.hpp>
#   include <Pothos/serialization/impl/mpl/if.hpp>
#   include <Pothos/serialization/impl/mpl/bool.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/msvc_is_class.hpp>
#elif POTHOS_WORKAROUND(__BORLANDC__, POTHOS_TESTED_AT(0x610))
#   include <Pothos/serialization/impl/mpl/if.hpp>
#   include <Pothos/serialization/impl/mpl/bool.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/yes_no.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/static_constant.hpp>
#   include <Pothos/serialization/impl/type_traits/is_class.hpp>
#else
#   include <Pothos/serialization/impl/mpl/aux_/type_wrapper.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/yes_no.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/static_constant.hpp>
#endif

namespace Pothos { namespace mpl { namespace aux {

#if POTHOS_WORKAROUND(__EDG_VERSION__, <= 244) && !defined(POTHOS_INTEL_CXX_VERSION)

POTHOS_MPL_HAS_XXX_TRAIT_NAMED_DEF(has_rebind, rebind, false)

#elif POTHOS_WORKAROUND(POTHOS_MSVC, < 1300)

POTHOS_MPL_HAS_XXX_TRAIT_NAMED_DEF(has_rebind_impl, rebind, false)

template< typename T >
struct has_rebind
    : if_< 
          msvc_is_class<T>
        , has_rebind_impl<T>
        , bool_<false>
        >::type
{
};

#else // the rest

template< typename T > struct has_rebind_tag {};
no_tag operator|(has_rebind_tag<int>, void const volatile*);

#   if !POTHOS_WORKAROUND(__BORLANDC__, POTHOS_TESTED_AT(0x610))
template< typename T >
struct has_rebind
{
    static has_rebind_tag<T>* get();
    POTHOS_STATIC_CONSTANT(bool, value = 
          sizeof(has_rebind_tag<int>() | get()) == sizeof(yes_tag)
        );
};
#   else // __BORLANDC__
template< typename T >
struct has_rebind_impl
{
    static T* get();
    POTHOS_STATIC_CONSTANT(bool, value = 
          sizeof(has_rebind_tag<int>() | get()) == sizeof(yes_tag)
        );
};

template< typename T >
struct has_rebind
    : if_< 
          is_class<T>
        , has_rebind_impl<T>
        , bool_<false>
        >::type
{
};
#   endif // __BORLANDC__

#endif

}}}

#endif // BOOST_MPL_AUX_HAS_REBIND_HPP_INCLUDED
