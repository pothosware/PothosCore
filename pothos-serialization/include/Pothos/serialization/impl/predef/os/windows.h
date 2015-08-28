/*
Copyright Redshift Software, Inc. 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef POTHOS_PREDEF_OS_WINDOWS_H
#define POTHOS_PREDEF_OS_WINDOWS_H

#include <Pothos/serialization/impl/predef/version_number.h>
#include <Pothos/serialization/impl/predef/make.h>

/*`
[heading `POTHOS_OS_WINDOWS`]

[@http://en.wikipedia.org/wiki/Category:Microsoft_Windows Microsoft Windows] operating system.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`_WIN32`] [__predef_detection__]]
    [[`_WIN64`] [__predef_detection__]]
    [[`__WIN32__`] [__predef_detection__]]
    [[`__TOS_WIN__`] [__predef_detection__]]
    [[`__WINDOWS__`] [__predef_detection__]]
    ]
 */

#define POTHOS_OS_WINDOWS POTHOS_VERSION_NUMBER_NOT_AVAILABLE

#if !POTHOS_PREDEF_DETAIL_OS_DETECTED && ( \
    defined(_WIN32) || defined(_WIN64) || \
    defined(__WIN32__) || defined(__TOS_WIN__) || \
    defined(__WINDOWS__) \
    )
#   undef POTHOS_OS_WINDOWS
#   define POTHOS_OS_WINDOWS POTHOS_VERSION_NUMBER_AVAILABLE
#endif

#if POTHOS_OS_WINDOWS
#   define POTHOS_OS_WINDOWS_AVAILABLE
#   include <Pothos/serialization/impl/predef/detail/os_detected.h>
#endif

#define POTHOS_OS_WINDOWS_NAME "Microsoft Windows"

#include <Pothos/serialization/impl/predef/detail/test.h>
POTHOS_PREDEF_DECLARE_TEST(POTHOS_OS_WINDOWS,POTHOS_OS_WINDOWS_NAME)

#endif
