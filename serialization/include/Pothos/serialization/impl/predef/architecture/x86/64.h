/*
Copyright Redshift Software, Inc. 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef POTHOS_PREDEF_ARCHITECTURE_X86_64_H
#define POTHOS_PREDEF_ARCHITECTURE_X86_64_H

#include <Pothos/serialization/impl/predef/version_number.h>
#include <Pothos/serialization/impl/predef/make.h>

/*`
[heading `POTHOS_ARCH_X86_64`]

[@http://en.wikipedia.org/wiki/Ia64 Intel IA-64] architecture.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__x86_64`] [__predef_detection__]]
    [[`__x86_64__`] [__predef_detection__]]
    [[`__amd64__`] [__predef_detection__]]
    [[`__amd64`] [__predef_detection__]]
    [[`_M_X64`] [__predef_detection__]]
    ]
 */

#define POTHOS_ARCH_X86_64 POTHOS_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__x86_64) || defined(__x86_64__) || \
    defined(__amd64__) || defined(__amd64) || \
    defined(_M_X64)
#   undef POTHOS_ARCH_X86_64
#   define POTHOS_ARCH_X86_64 POTHOS_VERSION_NUMBER_AVAILABLE
#endif

#if POTHOS_ARCH_X86_64
#   define POTHOS_ARCH_X86_64_AVAILABLE
#endif

#define POTHOS_ARCH_X86_64_NAME "Intel x86-64"

#include <Pothos/serialization/impl/predef/detail/test.h>
POTHOS_PREDEF_DECLARE_TEST(POTHOS_ARCH_X86_64,POTHOS_ARCH_X86_64_NAME)

#include <Pothos/serialization/impl/predef/architecture/x86.h>

#endif
