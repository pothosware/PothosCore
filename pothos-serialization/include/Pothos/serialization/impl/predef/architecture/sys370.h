/*
Copyright Redshift Software, Inc. 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef POTHOS_PREDEF_ARCHITECTURE_SYS370_H
#define POTHOS_PREDEF_ARCHITECTURE_SYS370_H

#include <Pothos/serialization/impl/predef/version_number.h>
#include <Pothos/serialization/impl/predef/make.h>

/*`
[heading `POTHOS_ARCH_SYS370`]

[@http://en.wikipedia.org/wiki/System/370 System/370] architecture.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__370__`] [__predef_detection__]]
    [[`__THW_370__`] [__predef_detection__]]
    ]
 */

#define POTHOS_ARCH_SYS370 POTHOS_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__370__) || defined(__THW_370__)
#   undef POTHOS_ARCH_SYS370
#   define POTHOS_ARCH_SYS370 POTHOS_VERSION_NUMBER_AVAILABLE
#endif

#if POTHOS_ARCH_SYS370
#   define POTHOS_ARCH_SYS370_AVAILABLE
#endif

#define POTHOS_ARCH_SYS370_NAME "System/370"

#include <Pothos/serialization/impl/predef/detail/test.h>
POTHOS_PREDEF_DECLARE_TEST(POTHOS_ARCH_SYS370,POTHOS_ARCH_SYS370_NAME)


#endif
