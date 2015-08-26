
#ifndef POTHOS_MPL_AUX_CONFIG_OVERLOAD_RESOLUTION_HPP_INCLUDED
#define POTHOS_MPL_AUX_CONFIG_OVERLOAD_RESOLUTION_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2002-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: overload_resolution.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/aux_/config/workaround.hpp>

#if    !defined(POTHOS_MPL_CFG_BROKEN_OVERLOAD_RESOLUTION) \
    && !defined(POTHOS_MPL_PREPROCESSING_MODE) \
    && (   POTHOS_WORKAROUND(__BORLANDC__, < 0x590) \
        || POTHOS_WORKAROUND(__MWERKS__, < 0x3001) \
        )

#   define POTHOS_MPL_CFG_BROKEN_OVERLOAD_RESOLUTION

#endif

#endif // BOOST_MPL_AUX_CONFIG_OVERLOAD_RESOLUTION_HPP_INCLUDED
