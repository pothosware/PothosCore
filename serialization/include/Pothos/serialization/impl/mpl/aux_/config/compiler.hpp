
#ifndef POTHOS_MPL_AUX_CONFIG_COMPILER_HPP_INCLUDED
#define POTHOS_MPL_AUX_CONFIG_COMPILER_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2001-2008
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: compiler.hpp 53189 2009-05-22 20:07:55Z hkaiser $
// $Date: 2009-05-22 13:07:55 -0700 (Fri, 22 May 2009) $
// $Revision: 53189 $

#if !defined(POTHOS_MPL_CFG_COMPILER_DIR)

#   include <Pothos/serialization/impl/mpl/aux_/config/dtp.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/ttp.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/ctps.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/msvc.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/gcc.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/workaround.hpp>

#   if POTHOS_WORKAROUND(POTHOS_MSVC, < 1300)
#       define POTHOS_MPL_CFG_COMPILER_DIR msvc60

#   elif POTHOS_WORKAROUND(POTHOS_MSVC, == 1300)
#       define POTHOS_MPL_CFG_COMPILER_DIR msvc70

#   elif POTHOS_WORKAROUND(POTHOS_MPL_CFG_GCC, POTHOS_TESTED_AT(0x0304))
#       define POTHOS_MPL_CFG_COMPILER_DIR gcc

#   elif POTHOS_WORKAROUND(__BORLANDC__, POTHOS_TESTED_AT(0x610))
#       if !defined(POTHOS_MPL_CFG_NO_DEFAULT_PARAMETERS_IN_NESTED_TEMPLATES)
#           define POTHOS_MPL_CFG_COMPILER_DIR bcc551
#       elif POTHOS_WORKAROUND(__BORLANDC__, >= 0x590)
#           define POTHOS_MPL_CFG_COMPILER_DIR bcc
#       else
#           define POTHOS_MPL_CFG_COMPILER_DIR bcc_pre590
#       endif

#   elif POTHOS_WORKAROUND(__DMC__, POTHOS_TESTED_AT(0x840))
#       define POTHOS_MPL_CFG_COMPILER_DIR dmc

#   elif defined(__MWERKS__)
#       if defined(POTHOS_MPL_CFG_BROKEN_DEFAULT_PARAMETERS_IN_NESTED_TEMPLATES)
#           define POTHOS_MPL_CFG_COMPILER_DIR mwcw
#       else
#           define POTHOS_MPL_CFG_COMPILER_DIR plain
#       endif

#   elif defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
#       define POTHOS_MPL_CFG_COMPILER_DIR no_ctps

#   elif defined(POTHOS_MPL_CFG_NO_TEMPLATE_TEMPLATE_PARAMETERS)
#       define POTHOS_MPL_CFG_COMPILER_DIR no_ttp

#   else
#       define POTHOS_MPL_CFG_COMPILER_DIR plain
#   endif

#endif // BOOST_MPL_CFG_COMPILER_DIR

#endif // BOOST_MPL_AUX_CONFIG_COMPILER_HPP_INCLUDED
