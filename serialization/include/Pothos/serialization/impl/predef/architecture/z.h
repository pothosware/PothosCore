/*
Copyright Redshift Software, Inc. 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef POTHOS_PREDEF_ARCHITECTURE_Z_H
#define POTHOS_PREDEF_ARCHITECTURE_Z_H

#include <Pothos/serialization/impl/predef/version_number.h>
#include <Pothos/serialization/impl/predef/make.h>

/*`
[heading `POTHOS_ARCH_Z`]

[@http://en.wikipedia.org/wiki/Z/Architecture z/Architecture] architecture.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__SYSC_ZARCH__`] [__predef_detection__]]
    ]
 */

#define POTHOS_ARCH_Z POTHOS_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__SYSC_ZARCH__)
#   undef POTHOS_ARCH_Z
#   define POTHOS_ARCH_Z POTHOS_VERSION_NUMBER_AVAILABLE
#endif

#if POTHOS_ARCH_Z
#   define POTHOS_ARCH_Z_AVAILABLE
#endif

#define POTHOS_ARCH_Z_NAME "z/Architecture"

#include <Pothos/serialization/impl/predef/detail/test.h>
POTHOS_PREDEF_DECLARE_TEST(POTHOS_ARCH_Z,POTHOS_ARCH_Z_NAME)


#endif
