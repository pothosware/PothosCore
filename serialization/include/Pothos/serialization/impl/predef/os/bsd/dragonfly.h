/*
Copyright Redshift Software, Inc. 2012-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef POTHOS_PREDEF_OS_BSD_DRAGONFLY_H
#define POTHOS_PREDEF_OS_BSD_DRAGONFLY_H

#include <Pothos/serialization/impl/predef/os/bsd.h>

/*`
[heading `POTHOS_OS_BSD_DRAGONFLY`]

[@http://en.wikipedia.org/wiki/DragonFly_BSD DragonFly BSD] operating system.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__DragonFly__`] [__predef_detection__]]
    ]
 */

#define POTHOS_OS_BSD_DRAGONFLY POTHOS_VERSION_NUMBER_NOT_AVAILABLE

#if !POTHOS_PREDEF_DETAIL_OS_DETECTED && ( \
    defined(__DragonFly__) \
    )
#   ifndef POTHOS_OS_BSD_AVAILABLE
#       define POTHOS_OS_BSD POTHOS_VERSION_NUMBER_AVAILABLE
#       define POTHOS_OS_BSD_AVAILABLE
#   endif
#   undef POTHOS_OS_BSD_DRAGONFLY
#   if defined(__DragonFly__)
#       define POTHOS_OS_DRAGONFLY_BSD POTHOS_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if POTHOS_OS_BSD_DRAGONFLY
#   define POTHOS_OS_BSD_DRAGONFLY_AVAILABLE
#   include <Pothos/serialization/impl/predef/detail/os_detected.h>
#endif

#define POTHOS_OS_BSD_DRAGONFLY_NAME "DragonFly BSD"

#include <Pothos/serialization/impl/predef/detail/test.h>
POTHOS_PREDEF_DECLARE_TEST(POTHOS_OS_BSD_DRAGONFLY,POTHOS_OS_BSD_DRAGONFLY_NAME)

#endif
