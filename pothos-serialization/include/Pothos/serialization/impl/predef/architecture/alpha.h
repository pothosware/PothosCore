/*
Copyright Redshift Software, Inc. 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef POTHOS_PREDEF_ARCHITECTURE_ALPHA_H
#define POTHOS_PREDEF_ARCHITECTURE_ALPHA_H

#include <Pothos/serialization/impl/predef/version_number.h>
#include <Pothos/serialization/impl/predef/make.h>

/*`
[heading `POTHOS_ARCH_ALPHA`]

[@http://en.wikipedia.org/wiki/DEC_Alpha DEC Alpha] architecture.

[table
    [[__predef_symbol__] [__predef_version__]]
    [[`__alpha__`] [__predef_detection__]]
    [[`__alpha`] [__predef_detection__]]
    [[`_M_ALPHA`] [__predef_detection__]]

    [[`__alpha_ev4__`] [4.0.0]]
    [[`__alpha_ev5__`] [5.0.0]]
    [[`__alpha_ev6__`] [6.0.0]]
    ]
 */

#define POTHOS_ARCH_ALPHA POTHOS_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__alpha__) || defined(__alpha) || \
    defined(_M_ALPHA)
#   undef POTHOS_ARCH_ALPHA
#   if !defined(POTHOS_ARCH_ALPHA) && defined(__alpha_ev4__)
#       define POTHOS_ARCH_ALPHA POTHOS_VERSION_NUMBER(4,0,0)
#   endif
#   if !defined(POTHOS_ARCH_ALPHA) && defined(__alpha_ev5__)
#       define POTHOS_ARCH_ALPHA POTHOS_VERSION_NUMBER(5,0,0)
#   endif
#   if !defined(POTHOS_ARCH_ALPHA) && defined(__alpha_ev6__)
#       define POTHOS_ARCH_ALPHA POTHOS_VERSION_NUMBER(6,0,0)
#   endif
#   if !defined(POTHOS_ARCH_ALPHA)
#       define POTHOS_ARCH_ALPHA POTHOS_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if POTHOS_ARCH_ALPHA
#   define POTHOS_ARCH_ALPHA_AVAILABLE
#endif

#define POTHOS_ARCH_ALPHA_NAME "DEC Alpha"

#include <Pothos/serialization/impl/predef/detail/test.h>
POTHOS_PREDEF_DECLARE_TEST(POTHOS_ARCH_ALPHA,POTHOS_ARCH_ALPHA_NAME)


#endif
