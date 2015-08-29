/*
Copyright Redshift Software, Inc. 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef POTHOS_PREDEF_ARCHITECTURE_PPC_H
#define POTHOS_PREDEF_ARCHITECTURE_PPC_H

#include <Pothos/serialization/impl/predef/version_number.h>
#include <Pothos/serialization/impl/predef/make.h>

/*`
[heading `POTHOS_ARCH_PPC`]

[@http://en.wikipedia.org/wiki/PowerPC PowerPC] architecture.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__powerpc`] [__predef_detection__]]
    [[`__powerpc__`] [__predef_detection__]]
    [[`__POWERPC__`] [__predef_detection__]]
    [[`__ppc__`] [__predef_detection__]]
    [[`_M_PPC`] [__predef_detection__]]
    [[`_ARCH_PPC`] [__predef_detection__]]
    [[`__PPCGECKO__`] [__predef_detection__]]
    [[`__PPCBROADWAY__`] [__predef_detection__]]
    [[`_XENON`] [__predef_detection__]]

    [[`__ppc601__`] [6.1.0]]
    [[`_ARCH_601`] [6.1.0]]
    [[`__ppc603__`] [6.3.0]]
    [[`_ARCH_603`] [6.3.0]]
    [[`__ppc604__`] [6.4.0]]
    [[`__ppc604__`] [6.4.0]]
    ]
 */

#define POTHOS_ARCH_PPC POTHOS_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__powerpc) || defined(__powerpc__) || \
    defined(__POWERPC__) || defined(__ppc__) || \
    defined(_M_PPC) || defined(_ARCH_PPC) || \
    defined(__PPCGECKO__) || defined(__PPCBROADWAY__) || \
    defined(_XENON)
#   undef POTHOS_ARCH_PPC
#   if !defined (POTHOS_ARCH_PPC) && (defined(__ppc601__) || defined(_ARCH_601))
#       define POTHOS_ARCH_PPC POTHOS_VERSION_NUMBER(6,1,0)
#   endif
#   if !defined (POTHOS_ARCH_PPC) && (defined(__ppc603__) || defined(_ARCH_603))
#       define POTHOS_ARCH_PPC POTHOS_VERSION_NUMBER(6,3,0)
#   endif
#   if !defined (POTHOS_ARCH_PPC) && (defined(__ppc604__) || defined(__ppc604__))
#       define POTHOS_ARCH_PPC POTHOS_VERSION_NUMBER(6,4,0)
#   endif
#   if !defined (POTHOS_ARCH_PPC)
#       define POTHOS_ARCH_PPC POTHOS_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if POTHOS_ARCH_PPC
#   define POTHOS_ARCH_PPC_AVAILABLE
#endif

#define POTHOS_ARCH_PPC_NAME "PowerPC"

#include <Pothos/serialization/impl/predef/detail/test.h>
POTHOS_PREDEF_DECLARE_TEST(POTHOS_ARCH_PPC,POTHOS_ARCH_PPC_NAME)


#endif
