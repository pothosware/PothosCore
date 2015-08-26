/*
Copyright Redshift Software, Inc. 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef POTHOS_PREDEF_ARCHITECTURE_RS6K_H
#define POTHOS_PREDEF_ARCHITECTURE_RS6K_H

#include <Pothos/serialization/impl/predef/version_number.h>
#include <Pothos/serialization/impl/predef/make.h>

/*`
[heading `POTHOS_ARCH_RS6000`]

[@http://en.wikipedia.org/wiki/RS/6000 RS/6000] architecture.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__THW_RS6000`] [__predef_detection__]]
    [[`_IBMR2`] [__predef_detection__]]
    [[`_POWER`] [__predef_detection__]]
    [[`_ARCH_PWR`] [__predef_detection__]]
    [[`_ARCH_PWR2`] [__predef_detection__]]
    ]
 */

#define POTHOS_ARCH_RS6000 POTHOS_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__THW_RS6000) || defined(_IBMR2) || \
    defined(_POWER) || defined(_ARCH_PWR) || \
    defined(_ARCH_PWR2)
#   undef POTHOS_ARCH_RS6000
#   define POTHOS_ARCH_RS6000 POTHOS_VERSION_NUMBER_AVAILABLE
#endif

#if POTHOS_ARCH_RS6000
#   define POTHOS_ARCH_RS6000_AVAILABLE
#endif

#define POTHOS_ARCH_RS6000_NAME "RS/6000"

#include <Pothos/serialization/impl/predef/detail/test.h>
POTHOS_PREDEF_DECLARE_TEST(POTHOS_ARCH_RS6000,POTHOS_ARCH_RS6000_NAME)

#define POTHOS_ARCH_PWR POTHOS_ARCH_RS6000

#if POTHOS_ARCH_PWR
#   define POTHOS_ARCH_PWR_AVAILABLE
#endif

#define POTHOS_ARCH_PWR_NAME POTHOS_ARCH_RS6000_NAME

#endif
