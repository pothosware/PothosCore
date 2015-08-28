/*
Copyright Redshift Software, Inc. 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef POTHOS_PREDEF_ARCHITECTURE_X86_H
#define POTHOS_PREDEF_ARCHITECTURE_X86_H

#include <Pothos/serialization/impl/predef/architecture/x86/32.h>
#include <Pothos/serialization/impl/predef/architecture/x86/64.h>

/*`
[heading `POTHOS_ARCH_X86`]

[@http://en.wikipedia.org/wiki/X86 Intel x86] architecture. This is
a category to indicate that either `POTHOS_ARCH_X86_32` or
`POTHOS_ARCH_X86_64` is detected.
 */

#define POTHOS_ARCH_X86 POTHOS_VERSION_NUMBER_NOT_AVAILABLE

#if POTHOS_ARCH_X86_32 || POTHOS_ARCH_X86_64
#   undef POTHOS_ARCH_X86
#   define POTHOS_ARCH_X86 POTHOS_VERSION_NUMBER_AVAILABLE
#endif

#if POTHOS_ARCH_X86
#   define POTHOS_ARCH_X86_AVAILABLE
#endif

#define POTHOS_ARCH_X86_NAME "Intel x86"

#include <Pothos/serialization/impl/predef/detail/test.h>
POTHOS_PREDEF_DECLARE_TEST(POTHOS_ARCH_X86,POTHOS_ARCH_X86_NAME)

#endif
