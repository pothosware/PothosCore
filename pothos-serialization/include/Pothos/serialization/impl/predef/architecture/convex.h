/*
Copyright Redshift Software Inc 2011-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef POTHOS_PREDEF_ARCHITECTURE_CONVEX_H
#define POTHOS_PREDEF_ARCHITECTURE_CONVEX_H

#include <Pothos/serialization/impl/predef/version_number.h>
#include <Pothos/serialization/impl/predef/make.h>

/*`
[heading `POTHOS_ARCH_CONVEX`]

[@http://en.wikipedia.org/wiki/Convex_Computer Convex Computer] architecture.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__convex__`] [__predef_detection__]]

    [[`__convex_c1__`] [1.0.0]]
    [[`__convex_c2__`] [2.0.0]]
    [[`__convex_c32__`] [3.2.0]]
    [[`__convex_c34__`] [3.4.0]]
    [[`__convex_c38__`] [3.8.0]]
    ]
 */

#define POTHOS_ARCH_CONVEX POTHOS_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__convex__)
#   undef POTHOS_ARCH_CONVEX
#   if !defined(POTHOS_ARCH_CONVEX) && defined(__convex_c1__)
#       define POTHOS_ARCH_CONVEX POTHOS_VERSION_NUMBER(1,0,0)
#   endif
#   if !defined(POTHOS_ARCH_CONVEX) && defined(__convex_c2__)
#       define POTHOS_ARCH_CONVEX POTHOS_VERSION_NUMBER(2,0,0)
#   endif
#   if !defined(POTHOS_ARCH_CONVEX) && defined(__convex_c32__)
#       define POTHOS_ARCH_CONVEX POTHOS_VERSION_NUMBER(3,2,0)
#   endif
#   if !defined(POTHOS_ARCH_CONVEX) && defined(__convex_c34__)
#       define POTHOS_ARCH_CONVEX POTHOS_VERSION_NUMBER(3,4,0)
#   endif
#   if !defined(POTHOS_ARCH_CONVEX) && defined(__convex_c38__)
#       define POTHOS_ARCH_CONVEX POTHOS_VERSION_NUMBER(3,8,0)
#   endif
#   if !defined(POTHOS_ARCH_CONVEX)
#       define POTHOS_ARCH_CONVEX POTHOS_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if POTHOS_ARCH_CONVEX
#   define POTHOS_ARCH_CONVEX_AVAILABLE
#endif

#define POTHOS_ARCH_CONVEX_NAME "Convex Computer"

#include <Pothos/serialization/impl/predef/detail/test.h>
POTHOS_PREDEF_DECLARE_TEST(POTHOS_ARCH_CONVEX,POTHOS_ARCH_CONVEX_NAME)



#endif
