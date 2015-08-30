/*
Copyright Redshift Software, Inc. 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef POTHOS_PREDEF_ARCHITECTURE_SPARC_H
#define POTHOS_PREDEF_ARCHITECTURE_SPARC_H

#include <Pothos/serialization/impl/predef/version_number.h>
#include <Pothos/serialization/impl/predef/make.h>

/*`
[heading `POTHOS_ARCH_SPARC`]

[@http://en.wikipedia.org/wiki/SPARC SPARC] architecture.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__sparc__`] [__predef_detection__]]
    [[`__sparc`] [__predef_detection__]]

    [[`__sparcv9`] [9.0.0]]
    [[`__sparcv8`] [8.0.0]]
    ]
 */

#define POTHOS_ARCH_SPARC POTHOS_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__sparc__) || defined(__sparc)
#   undef POTHOS_ARCH_SPARC
#   if !defined(POTHOS_ARCH_SPARC) && defined(__sparcv9)
#       define POTHOS_ARCH_SPARC POTHOS_VERSION_NUMBER(9,0,0)
#   endif
#   if !defined(POTHOS_ARCH_SPARC) &&  defined(__sparcv8)
#       define POTHOS_ARCH_SPARC POTHOS_VERSION_NUMBER(8,0,0)
#   endif
#   if !defined(POTHOS_ARCH_SPARC) &&
#       define POTHOS_ARCH_SPARC POTHOS_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if POTHOS_ARCH_SPARC
#   define POTHOS_ARCH_SPARC_AVAILABLE
#endif

#define POTHOS_ARCH_SPARC_NAME "SPARC"

#include <Pothos/serialization/impl/predef/detail/test.h>
POTHOS_PREDEF_DECLARE_TEST(POTHOS_ARCH_SPARC,POTHOS_ARCH_SPARC_NAME)


#endif
