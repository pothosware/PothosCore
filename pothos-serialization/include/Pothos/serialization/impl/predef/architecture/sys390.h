/*
Copyright Redshift Software, Inc. 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef POTHOS_PREDEF_ARCHITECTURE_SYS390_H
#define POTHOS_PREDEF_ARCHITECTURE_SYS390_H

#include <Pothos/serialization/impl/predef/version_number.h>
#include <Pothos/serialization/impl/predef/make.h>

/*`
[heading `POTHOS_ARCH_SYS390`]

[@http://en.wikipedia.org/wiki/System/390 System/390] architecture.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__s390__`] [__predef_detection__]]
    [[`__s390x__`] [__predef_detection__]]
    ]
 */

#define POTHOS_ARCH_SYS390 POTHOS_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__s390__) || defined(__s390x__)
#   undef POTHOS_ARCH_SYS390
#   define POTHOS_ARCH_SYS390 POTHOS_VERSION_NUMBER_AVAILABLE
#endif

#if POTHOS_ARCH_SYS390
#   define POTHOS_ARCH_SYS390_AVAILABLE
#endif

#define POTHOS_ARCH_SYS390_NAME "System/390"

#include <Pothos/serialization/impl/predef/detail/test.h>
POTHOS_PREDEF_DECLARE_TEST(POTHOS_ARCH_SYS390,POTHOS_ARCH_SYS390_NAME)


#endif
