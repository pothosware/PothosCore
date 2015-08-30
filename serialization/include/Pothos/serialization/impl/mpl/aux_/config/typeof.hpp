
#ifndef POTHOS_MPL_AUX_CONFIG_TYPEOF_HPP_INCLUDED
#define POTHOS_MPL_AUX_CONFIG_TYPEOF_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2003-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: typeof.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/aux_/config/gcc.hpp>

#if !defined(POTHOS_MPL_CFG_HAS_TYPEOF) \
    && !defined(POTHOS_MPL_PREPROCESSING_MODE) \
    && (   defined(POTHOS_MPL_CFG_GCC) && POTHOS_MPL_CFG_GCC >= 0x0302 \
        || defined(__MWERKS__) && __MWERKS__ >= 0x3000 \
        )

#   define POTHOS_MPL_CFG_HAS_TYPEOF

#endif


#if !defined(POTHOS_MPL_CFG_TYPEOF_BASED_SEQUENCES) \
    && !defined(POTHOS_MPL_PREPROCESSING_MODE) \
    && defined(POTHOS_MPL_CFG_HAS_TYPEOF)

#   define POTHOS_MPL_CFG_TYPEOF_BASED_SEQUENCES

#endif

#endif // BOOST_MPL_AUX_CONFIG_TYPEOF_HPP_INCLUDED
