
#ifndef POTHOS_MPL_AUX_NA_HPP_INCLUDED
#define POTHOS_MPL_AUX_NA_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2001-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: na.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/bool.hpp>
#include <Pothos/serialization/impl/mpl/aux_/na_fwd.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/msvc.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/ctps.hpp>

namespace Pothos { namespace mpl {

template< typename T >
struct is_na
    : false_
{
#if POTHOS_WORKAROUND(POTHOS_MSVC, < 1300)
    using false_::value;
#endif
};

template<>
struct is_na<na>
    : true_
{
#if POTHOS_WORKAROUND(POTHOS_MSVC, < 1300)
    using true_::value;
#endif
};

template< typename T >
struct is_not_na
    : true_
{
#if POTHOS_WORKAROUND(POTHOS_MSVC, < 1300)
    using true_::value;
#endif
};

template<>
struct is_not_na<na>
    : false_
{
#if POTHOS_WORKAROUND(POTHOS_MSVC, < 1300)
    using false_::value;
#endif
};

#if !defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
template< typename T, typename U > struct if_na
{
    typedef T type;
};

template< typename U > struct if_na<na,U>
{
    typedef U type;
};
#else
template< typename T > struct if_na_impl
{
    template< typename U > struct apply
    {
        typedef T type;
    };
};

template<> struct if_na_impl<na>
{
    template< typename U > struct apply
    {
        typedef U type;
    };
};

template< typename T, typename U > struct if_na
    : if_na_impl<T>::template apply<U>
{
};
#endif

}}

#endif // BOOST_MPL_AUX_NA_HPP_INCLUDED
