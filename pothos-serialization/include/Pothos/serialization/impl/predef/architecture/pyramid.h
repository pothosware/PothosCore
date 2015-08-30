/*
Copyright Redshift Software Inc 2011-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef POTHOS_PREDEF_ARCHITECTURE_PYRAMID_H
#define POTHOS_PREDEF_ARCHITECTURE_PYRAMID_H

#include <Pothos/serialization/impl/predef/version_number.h>
#include <Pothos/serialization/impl/predef/make.h>

/*`
[heading `POTHOS_ARCH_PYRAMID`]

Pyramid 9810 architecture.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`pyr`] [__predef_detection__]]
    ]
 */

#define POTHOS_ARCH_PYRAMID POTHOS_VERSION_NUMBER_NOT_AVAILABLE

#if defined(pyr)
#   undef POTHOS_ARCH_PYRAMID
#   define POTHOS_ARCH_PYRAMID POTHOS_VERSION_NUMBER_AVAILABLE
#endif

#if POTHOS_ARCH_PYRAMID
#   define POTHOS_ARCH_PYRAMID_AVAILABLE
#endif

#define POTHOS_ARCH_PYRAMID_NAME "Pyramid 9810"

#include <Pothos/serialization/impl/predef/detail/test.h>
POTHOS_PREDEF_DECLARE_TEST(POTHOS_ARCH_PYRAMID,POTHOS_ARCH_PYRAMID_NAME)


#endif
