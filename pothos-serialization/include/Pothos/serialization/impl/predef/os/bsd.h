/*
Copyright Redshift Software, Inc. 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef POTHOS_PREDEF_OS_BSD_H
#define POTHOS_PREDEF_OS_BSD_H

/* Special case: OSX will define BSD predefs if the sys/param.h
 * header is included. We can guard against that, but only if we
 * detect OSX first. Hence we will force include OSX detection
 * before doing any BSD detection.
 */
#include <Pothos/serialization/impl/predef/os/macos.h>

#include <Pothos/serialization/impl/predef/version_number.h>
#include <Pothos/serialization/impl/predef/make.h>

/*`
[heading `POTHOS_OS_BSD`]

[@http://en.wikipedia.org/wiki/Berkeley_Software_Distribution BSD] operating system.

BSD has various branch operating systems possible and each detected
individually. This detects the following variations and sets a specific
version number macro to match:

* `POTHOS_OS_BSD_DRAGONFLY` [@http://en.wikipedia.org/wiki/DragonFly_BSD DragonFly BSD]
* `POTHOS_OS_BSD_FREE` [@http://en.wikipedia.org/wiki/Freebsd FreeBSD]
* `POTHOS_OS_BSD_BSDI` [@http://en.wikipedia.org/wiki/BSD/OS BSDi BSD/OS]
* `POTHOS_OS_BSD_NET` [@http://en.wikipedia.org/wiki/Netbsd NetBSD]
* `POTHOS_OS_BSD_OPEN` [@http://en.wikipedia.org/wiki/Openbsd OpenBSD]

[note The general `POTHOS_OS_BSD` is set in all cases to indicate some form
of BSD. If the above variants is detected the corresponding macro is also set.]

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`BSD`] [__predef_detection__]]
    [[`_SYSTYPE_BSD`] [__predef_detection__]]

    [[`BSD4_2`] [4.2.0]]
    [[`BSD4_3`] [4.3.0]]
    [[`BSD4_4`] [4.4.0]]
    [[`BSD`] [V.R.0]]
    ]
 */

#include <Pothos/serialization/impl/predef/os/bsd/bsdi.h>
#include <Pothos/serialization/impl/predef/os/bsd/dragonfly.h>
#include <Pothos/serialization/impl/predef/os/bsd/free.h>
#include <Pothos/serialization/impl/predef/os/bsd/open.h>
#include <Pothos/serialization/impl/predef/os/bsd/net.h>

#ifndef POTHOS_OS_BSD
#define POTHOS_OS_BSD POTHOS_VERSION_NUMBER_NOT_AVAILABLE
#endif

#if !POTHOS_PREDEF_DETAIL_OS_DETECTED && ( \
    defined(BSD) || \
    defined(_SYSTYPE_BSD) \
    )
#   undef POTHOS_OS_BSD
#   include <sys/param.h>
#   if !defined(POTHOS_OS_BSD) && defined(BSD4_4)
#       define POTHOS_OS_BSD POTHOS_VERSION_NUMBER(4,4,0)
#   endif
#   if !defined(POTHOS_OS_BSD) && defined(BSD4_3)
#       define POTHOS_OS_BSD POTHOS_VERSION_NUMBER(4,3,0)
#   endif
#   if !defined(POTHOS_OS_BSD) && defined(BSD4_2)
#       define POTHOS_OS_BSD POTHOS_VERSION_NUMBER(4,2,0)
#   endif
#   if !defined(POTHOS_OS_BSD) && defined(BSD)
#       define POTHOS_OS_BSD POTHOS_PREDEF_MAKE_10_VVRR(BSD)
#   endif
#   if !defined(POTHOS_OS_BSD)
#       define POTHOS_OS_BSD POTHOS_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if POTHOS_OS_BSD
#   define POTHOS_OS_BSD_AVAILABLE
#   include <Pothos/serialization/impl/predef/detail/os_detected.h>
#endif

#define POTHOS_OS_BSD_NAME "BSD"

#include <Pothos/serialization/impl/predef/detail/test.h>
POTHOS_PREDEF_DECLARE_TEST(POTHOS_OS_BSD,POTHOS_OS_BSD_NAME)

#endif
