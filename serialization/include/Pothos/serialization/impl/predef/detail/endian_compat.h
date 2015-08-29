/*
Copyright Redshift Software, Inc. 2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef POTHOS_PREDEF_DETAIL_ENDIAN_COMPAT_H
#define POTHOS_PREDEF_DETAIL_ENDIAN_COMPAT_H

#include <Pothos/serialization/impl/predef/other/endian.h>

#if POTHOS_ENDIAN_BIG_BYTE
#   define POTHOS_BIG_ENDIAN
#   define POTHOS_BYTE_ORDER 4321
#endif
#if POTHOS_ENDIAN_LITTLE_BYTE
#   define POTHOS_LITTLE_ENDIAN
#   define POTHOS_BYTE_ORDER 1234
#endif
#if POTHOS_ENDIAN_LITTLE_WORD
#   define POTHOS_PDP_ENDIAN
#   define POTHOS_BYTE_ORDER 2134
#endif

#endif
