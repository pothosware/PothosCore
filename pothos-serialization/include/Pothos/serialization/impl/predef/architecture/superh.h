/*
Copyright Redshift Software, Inc. 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef POTHOS_PREDEF_ARCHITECTURE_SUPERH_H
#define POTHOS_PREDEF_ARCHITECTURE_SUPERH_H

#include <Pothos/serialization/impl/predef/version_number.h>
#include <Pothos/serialization/impl/predef/make.h>

/*`
[heading `POTHOS_ARCH_SH`]

[@http://en.wikipedia.org/wiki/SuperH SuperH] architecture:
If available versions \[1-5\] are specifically detected.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__sh__`] [__predef_detection__]]

    [[`__SH5__`] [5.0.0]]
    [[`__SH4__`] [4.0.0]]
    [[`__sh3__`] [3.0.0]]
    [[`__SH3__`] [3.0.0]]
    [[`__sh2__`] [2.0.0]]
    [[`__sh1__`] [1.0.0]]
    ]
 */

#define POTHOS_ARCH_SH POTHOS_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__sh__)
#   undef POTHOS_ARCH_SH
#   if !defined(POTHOS_ARCH_SH) && (defined(__SH5__))
#       define POTHOS_ARCH_SH POTHOS_VERSION_NUMBER(5,0,0)
#   endif
#   if !defined(POTHOS_ARCH_SH) && (defined(__SH4__))
#       define POTHOS_ARCH_SH POTHOS_VERSION_NUMBER(4,0,0)
#   endif
#   if !defined(POTHOS_ARCH_SH) && (defined(__sh3__) || defined(__SH3__))
#       define POTHOS_ARCH_SH POTHOS_VERSION_NUMBER(3,0,0)
#   endif
#   if !defined(POTHOS_ARCH_SH) && (defined(__sh2__))
#       define POTHOS_ARCH_SH POTHOS_VERSION_NUMBER(2,0,0)
#   endif
#   if !defined(POTHOS_ARCH_SH) && (defined(__sh1__))
#       define POTHOS_ARCH_SH POTHOS_VERSION_NUMBER(1,0,0)
#   endif
#   if !defined(POTHOS_ARCH_SH)
#       define POTHOS_ARCH_SH POTHOS_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if POTHOS_ARCH_SH
#   define POTHOS_ARCH_SH_AVAILABLE
#endif

#define POTHOS_ARCH_SH_NAME "SuperH"

#include <Pothos/serialization/impl/predef/detail/test.h>
POTHOS_PREDEF_DECLARE_TEST(POTHOS_ARCH_SH,POTHOS_ARCH_SH_NAME)


#endif
