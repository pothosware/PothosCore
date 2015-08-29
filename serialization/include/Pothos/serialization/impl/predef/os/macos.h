/*
Copyright Redshift Software, Inc. 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef POTHOS_PREDEF_OS_MACOS_H
#define POTHOS_PREDEF_OS_MACOS_H

#include <Pothos/serialization/impl/predef/version_number.h>
#include <Pothos/serialization/impl/predef/make.h>

/*`
[heading `POTHOS_OS_MACOS`]

[@http://en.wikipedia.org/wiki/Mac_OS Mac OS] operating system.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`macintosh`] [__predef_detection__]]
    [[`Macintosh`] [__predef_detection__]]
    [[`__APPLE__`] [__predef_detection__]]
    [[`__MACH__`] [__predef_detection__]]

    [[`__APPLE__`, `__MACH__`] [10.0.0]]
    [[ /otherwise/ ] [9.0.0]]
    ]
 */

#define POTHOS_OS_MACOS POTHOS_VERSION_NUMBER_NOT_AVAILABLE

#if !POTHOS_PREDEF_DETAIL_OS_DETECTED && ( \
    defined(macintosh) || defined(Macintosh) || \
    (defined(__APPLE__) && defined(__MACH__)) \
    )
#   undef POTHOS_OS_MACOS
#   if !defined(POTHOS_OS_MACOS) && defined(__APPLE__) && defined(__MACH__)
#       define POTHOS_OS_MACOS POTHOS_VERSION_NUMBER(10,0,0)
#   endif
#   if !defined(POTHOS_OS_MACOS)
#       define POTHOS_OS_MACOS POTHOS_VERSION_NUMBER(9,0,0)
#   endif
#endif

#if POTHOS_OS_MACOS
#   define POTHOS_OS_MACOS_AVAILABLE
#   include <Pothos/serialization/impl/predef/detail/os_detected.h>
#endif

#define POTHOS_OS_MACOS_NAME "Mac OS"

#include <Pothos/serialization/impl/predef/detail/test.h>
POTHOS_PREDEF_DECLARE_TEST(POTHOS_OS_MACOS,POTHOS_OS_MACOS_NAME)


#endif
