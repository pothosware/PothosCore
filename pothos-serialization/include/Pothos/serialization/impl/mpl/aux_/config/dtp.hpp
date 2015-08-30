
#ifndef POTHOS_MPL_AUX_CONFIG_DTP_HPP_INCLUDED
#define POTHOS_MPL_AUX_CONFIG_DTP_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2001-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: dtp.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/aux_/config/workaround.hpp>

// MWCW 7.x-8.0 "losts" default template parameters of nested class 
// templates when their owner classes are passed as arguments to other 
// templates; Borland 5.5.1 "forgets" them from the very beginning (if 
// the owner class is a class template), and Borland 5.6 isn't even
// able to compile a definition of nested class template with DTP

#if    !defined(POTHOS_MPL_CFG_NO_DEFAULT_PARAMETERS_IN_NESTED_TEMPLATES) \
    && !defined(POTHOS_MPL_PREPROCESSING_MODE) \
    && POTHOS_WORKAROUND(__BORLANDC__, >= 0x560) \
    && POTHOS_WORKAROUND(__BORLANDC__, POTHOS_TESTED_AT(0x610))

#   define POTHOS_MPL_CFG_NO_DEFAULT_PARAMETERS_IN_NESTED_TEMPLATES

#endif


#if    !defined(POTHOS_MPL_CFG_BROKEN_DEFAULT_PARAMETERS_IN_NESTED_TEMPLATES) \
    && !defined(POTHOS_MPL_PREPROCESSING_MODE) \
    && (   POTHOS_WORKAROUND(__MWERKS__, <= 0x3001) \
        || POTHOS_WORKAROUND(__BORLANDC__, POTHOS_TESTED_AT(0x610)) \
        || defined(POTHOS_MPL_CFG_NO_DEFAULT_PARAMETERS_IN_NESTED_TEMPLATES) \
        )
        
#   define POTHOS_MPL_CFG_BROKEN_DEFAULT_PARAMETERS_IN_NESTED_TEMPLATES

#endif

#endif // BOOST_MPL_AUX_CONFIG_DTP_HPP_INCLUDED
