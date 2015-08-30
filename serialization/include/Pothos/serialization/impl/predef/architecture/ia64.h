/*
Copyright Redshift Software, Inc. 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef POTHOS_PREDEF_ARCHITECTURE_IA64_H
#define POTHOS_PREDEF_ARCHITECTURE_IA64_H

#include <Pothos/serialization/impl/predef/version_number.h>
#include <Pothos/serialization/impl/predef/make.h>

/*`
[heading `POTHOS_ARCH_IA64`]

[@http://en.wikipedia.org/wiki/Ia64 Intel Itanium 64] architecture.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__ia64__`] [__predef_detection__]]
    [[`_IA64`] [__predef_detection__]]
    [[`__IA64__`] [__predef_detection__]]
    [[`__ia64`] [__predef_detection__]]
    [[`_M_IA64`] [__predef_detection__]]
    [[`__itanium__`] [__predef_detection__]]
    ]
 */

#define POTHOS_ARCH_IA64 POTHOS_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__ia64__) || defined(_IA64) || \
    defined(__IA64__) || defined(__ia64) || \
    defined(_M_IA64) || defined(__itanium__)
#   undef POTHOS_ARCH_IA64
#   define POTHOS_ARCH_IA64 POTHOS_VERSION_NUMBER_AVAILABLE
#endif

#if POTHOS_ARCH_IA64
#   define POTHOS_ARCH_IA64_AVAILABLE
#endif

#define POTHOS_ARCH_IA64_NAME "Intel Itanium 64"

#include <Pothos/serialization/impl/predef/detail/test.h>
POTHOS_PREDEF_DECLARE_TEST(POTHOS_ARCH_IA64,POTHOS_ARCH_IA64_NAME)

#endif
