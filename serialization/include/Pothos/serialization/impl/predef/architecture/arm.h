/*
Copyright Redshift Software, Inc. 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef POTHOS_PREDEF_ARCHITECTURE_ARM_H
#define POTHOS_PREDEF_ARCHITECTURE_ARM_H

#include <Pothos/serialization/impl/predef/version_number.h>
#include <Pothos/serialization/impl/predef/make.h>

/*`
[heading `POTHOS_ARCH_ARM`]

[@http://en.wikipedia.org/wiki/ARM_architecture ARM] architecture.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__arm__`] [__predef_detection__]]
    [[`__thumb__`] [__predef_detection__]]
    [[`__TARGET_ARCH_ARM`] [__predef_detection__]]
    [[`__TARGET_ARCH_THUMB`] [__predef_detection__]]

    [[`__TARGET_ARCH_ARM`] [V.0.0]]
    [[`__TARGET_ARCH_THUMB`] [V.0.0]]
    ]
 */

#define POTHOS_ARCH_ARM POTHOS_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__arm__) || defined(__thumb__) || \
    defined(__TARGET_ARCH_ARM) || defined(__TARGET_ARCH_THUMB)
#   undef POTHOS_ARCH_ARM
#   if !defined(POTHOS_ARCH_ARM) && defined(__TARGET_ARCH_ARM)
#       define POTHOS_ARCH_ARM POTHOS_VERSION_NUMBER(__TARGET_ARCH_ARM,0,0)
#   endif
#   if !defined(POTHOS_ARCH_ARM) && defined(__TARGET_ARCH_THUMB)
#       define POTHOS_ARCH_ARM POTHOS_VERSION_NUMBER(__TARGET_ARCH_THUMB,0,0)
#   endif
#   if !defined(POTHOS_ARCH_ARM)
#       define POTHOS_ARCH_ARM POTHOS_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if POTHOS_ARCH_ARM
#   define POTHOS_ARCH_ARM_AVAILABLE
#endif

#define POTHOS_ARCH_ARM_NAME "ARM"

#include <Pothos/serialization/impl/predef/detail/test.h>
POTHOS_PREDEF_DECLARE_TEST(POTHOS_ARCH_ARM,POTHOS_ARCH_ARM_NAME)


#endif
