
#ifndef POTHOS_MPL_VOID_HPP_INCLUDED
#define POTHOS_MPL_VOID_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2001-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: void.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/void_fwd.hpp>
#include <Pothos/serialization/impl/mpl/bool.hpp>
#include <Pothos/serialization/impl/mpl/aux_/na_spec.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/msvc.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/workaround.hpp>

POTHOS_MPL_AUX_ADL_BARRIER_NAMESPACE_OPEN

//  [JDG Feb-4-2003] made void_ a complete type to allow it to be
//  instantiated so that it can be passed in as an object that can be
//  used to select an overloaded function. Possible use includes signaling
//  a zero arity functor evaluation call.
struct void_ { typedef void_ type; };

POTHOS_MPL_AUX_ADL_BARRIER_NAMESPACE_CLOSE

namespace Pothos { namespace mpl {

template< typename T >
struct is_void_
    : false_
{
#if POTHOS_WORKAROUND(POTHOS_MSVC, < 1300)
    using false_::value;
#endif
};

template<>
struct is_void_<void_>
    : true_
{
#if POTHOS_WORKAROUND(POTHOS_MSVC, < 1300)
    using true_::value;
#endif
};

template< typename T >
struct is_not_void_
    : true_
{
#if POTHOS_WORKAROUND(POTHOS_MSVC, < 1300)
    using true_::value;
#endif
};

template<>
struct is_not_void_<void_>
    : false_
{
#if POTHOS_WORKAROUND(POTHOS_MSVC, < 1300)
    using false_::value;
#endif
};

POTHOS_MPL_AUX_NA_SPEC(1, is_void_)
POTHOS_MPL_AUX_NA_SPEC(1, is_not_void_)

}}

#endif // BOOST_MPL_VOID_HPP_INCLUDED
