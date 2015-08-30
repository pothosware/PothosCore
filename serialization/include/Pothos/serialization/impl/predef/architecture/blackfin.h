/*
Copyright Redshift Software Inc 2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef POTHOS_PREDEF_ARCHITECTURE_BLACKFIN_H
#define POTHOS_PREDEF_ARCHITECTURE_BLACKFIN_H

#include <Pothos/serialization/impl/predef/version_number.h>
#include <Pothos/serialization/impl/predef/make.h>

/*`
[heading `POTHOS_ARCH_BLACKFIN`]

Blackfin Processors from Analog Devices.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__bfin__`] [__predef_detection__]]
    [[`__BFIN__`] [__predef_detection__]]
    [[`bfin`] [__predef_detection__]]
    [[`BFIN`] [__predef_detection__]]
    ]
 */

#define POTHOS_ARCH_BLACKFIN POTHOS_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__bfin__) || defined(__BFIN__) || \
    defined(bfin) || defined(BFIN)
#   undef POTHOS_ARCH_BLACKFIN
#   define POTHOS_ARCH_BLACKFIN POTHOS_VERSION_NUMBER_AVAILABLE
#endif

#if POTHOS_ARCH_BLACKFIN
#   define POTHOS_ARCH_BLACKFIN_AVAILABLE
#endif

#define POTHOS_ARCH_BLACKFIN_NAME "Blackfin"

#include <Pothos/serialization/impl/predef/detail/test.h>
POTHOS_PREDEF_DECLARE_TEST(POTHOS_ARCH_BLACKFIN,POTHOS_ARCH_BLACKFIN_NAME)


#endif
