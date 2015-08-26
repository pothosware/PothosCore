/*
Copyright Redshift Software, Inc. 2012-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef POTHOS_PREDEF_OS_BSD_BSDI_H
#define POTHOS_PREDEF_OS_BSD_BSDI_H

#include <Pothos/serialization/impl/predef/os/bsd.h>

/*`
[heading `POTHOS_OS_BSD_BSDI`]

[@http://en.wikipedia.org/wiki/BSD/OS BSDi BSD/OS] operating system.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__bsdi__`] [__predef_detection__]]
    ]
 */

#define POTHOS_OS_BSD_BSDI POTHOS_VERSION_NUMBER_NOT_AVAILABLE

#if !POTHOS_PREDEF_DETAIL_OS_DETECTED && ( \
    defined(__bsdi__) \
    )
#   ifndef POTHOS_OS_BSD_AVAILABLE
#       define POTHOS_OS_BSD POTHOS_VERSION_NUMBER_AVAILABLE
#       define POTHOS_OS_BSD_AVAILABLE
#   endif
#   undef POTHOS_OS_BSD_BSDI
#   define POTHOS_OS_BSD_BSDI POTHOS_VERSION_NUMBER_AVAILABLE
#endif

#if POTHOS_OS_BSD_BSDI
#   define POTHOS_OS_BSD_BSDI_AVAILABLE
#   include <Pothos/serialization/impl/predef/detail/os_detected.h>
#endif

#define POTHOS_OS_BSD_BSDI_NAME "BSDi BSD/OS"

#include <Pothos/serialization/impl/predef/detail/test.h>
POTHOS_PREDEF_DECLARE_TEST(POTHOS_OS_BSD_BSDI,POTHOS_OS_BSD_BSDI_NAME)

#endif
