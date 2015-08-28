
#ifndef POTHOS_MPL_AUX_CONFIG_DEPENDENT_NTTP_HPP_INCLUDED
#define POTHOS_MPL_AUX_CONFIG_DEPENDENT_NTTP_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2002-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: dependent_nttp.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/aux_/config/gcc.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/workaround.hpp>

// GCC and EDG-based compilers incorrectly reject the following code:
//   template< typename T, T n > struct a;
//   template< typename T > struct b;
//   template< typename T, T n > struct b< a<T,n> > {};

#if    !defined(POTHOS_MPL_CFG_NO_DEPENDENT_NONTYPE_PARAMETER_IN_PARTIAL_SPEC) \
    && !defined(POTHOS_MPL_PREPROCESSING_MODE) \
    && (   POTHOS_WORKAROUND(__EDG_VERSION__, POTHOS_TESTED_AT(300)) \
        || POTHOS_WORKAROUND(POTHOS_MPL_CFG_GCC, POTHOS_TESTED_AT(0x0302)) \
        )

#   define POTHOS_MPL_CFG_NO_DEPENDENT_NONTYPE_PARAMETER_IN_PARTIAL_SPEC

#endif

#endif // BOOST_MPL_AUX_CONFIG_DEPENDENT_NTTP_HPP_INCLUDED
