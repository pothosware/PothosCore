/*
Copyright Redshift Software, Inc. 2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef POTHOS_PREDEF_ENDIAN_H
#define POTHOS_PREDEF_ENDIAN_H

#include <Pothos/serialization/impl/predef/version_number.h>
#include <Pothos/serialization/impl/predef/make.h>
#include <Pothos/serialization/impl/predef/library/c/gnu.h>
#include <Pothos/serialization/impl/predef/os/macos.h>
#include <Pothos/serialization/impl/predef/os/bsd.h>

/*`
[heading `POTHOS_ENDIAN_*`]

Detection of endian memory ordering. There are four defined macros
in this header that define the various generally possible endian
memory orderings:

* `POTHOS_ENDIAN_BIG_BYTE`, byte-swapped big-endian.
* `POTHOS_ENDIAN_BIG_WORD`, word-swapped big-endian.
* `POTHOS_ENDIAN_LITTLE_BYTE`, byte-swapped little-endian.
* `POTHOS_ENDIAN_LITTLE_WORD`, word-swapped little-endian.

The detection is conservative in that it only identifies endianness
that it knows for certain. In particular bi-endianness is not
indicated as is it not practically possible to determine the
endianness from anything but an operating system provided
header. And the currently known headers do not define that
programatic bi-endianness is available.

This implementation is a compilation of various publicly available
information and acquired knowledge:

# The indispensable documentation of "Pre-defined Compiler Macros"
  [@http://sourceforge.net/p/predef/wiki/Endianness Endianness].
# The various endian specifications available in the
  [@http://wikipedia.org/ Wikipedia] computer architecture pages.
# Generally available searches for headers that define endianness.
 */

#define POTHOS_ENDIAN_BIG_BYTE POTHOS_VERSION_NUMBER_NOT_AVAILABLE
#define POTHOS_ENDIAN_BIG_WORD POTHOS_VERSION_NUMBER_NOT_AVAILABLE
#define POTHOS_ENDIAN_LITTLE_BYTE POTHOS_VERSION_NUMBER_NOT_AVAILABLE
#define POTHOS_ENDIAN_LITTLE_WORD POTHOS_VERSION_NUMBER_NOT_AVAILABLE

/* GNU libc provides a header defining __BYTE_ORDER, or _BYTE_ORDER.
 * And some OSs provide some for of endian header also.
 */
#if !POTHOS_ENDIAN_BIG_BYTE && !POTHOS_ENDIAN_BIG_WORD && \
    !POTHOS_ENDIAN_LITTLE_BYTE && !POTHOS_ENDIAN_LITTLE_WORD
#   if POTHOS_LIB_C_GNU
#       include <endian.h>
#   else
#       if POTHOS_OS_MACOS
#           include <machine/endian.h>
#       else
#           if POTHOS_OS_BSD
#               if POTHOS_OS_BSD_OPEN
#                   include <machine/endian.h>
#               else
#                   include <sys/endian.h>
#               endif
#           endif
#       endif
#   endif
#   if defined(__BYTE_ORDER)
#       if (__BYTE_ORDER == __BIG_ENDIAN)
#           undef POTHOS_ENDIAN_BIG_BYTE
#           define POTHOS_ENDIAN_BIG_BYTE POTHOS_VERSION_NUMBER_AVAILABLE
#       endif
#       if (__BYTE_ORDER == __LITTLE_ENDIAN)
#           undef POTHOS_ENDIAN_LITTLE_BYTE
#           define POTHOS_ENDIAN_LITTLE_BYTE POTHOS_VERSION_NUMBER_AVAILABLE
#       endif
#       if (__BYTE_ORDER == __PDP_ENDIAN)
#           undef POTHOS_ENDIAN_LITTLE_WORD
#           define POTHOS_ENDIAN_LITTLE_WORD POTHOS_VERSION_NUMBER_AVAILABLE
#       endif
#   endif
#   if !defined(__BYTE_ORDER) && defined(_BYTE_ORDER)
#       if (_BYTE_ORDER == _BIG_ENDIAN)
#           undef POTHOS_ENDIAN_BIG_BYTE
#           define POTHOS_ENDIAN_BIG_BYTE POTHOS_VERSION_NUMBER_AVAILABLE
#       endif
#       if (_BYTE_ORDER == _LITTLE_ENDIAN)
#           undef POTHOS_ENDIAN_LITTLE_BYTE
#           define POTHOS_ENDIAN_LITTLE_BYTE POTHOS_VERSION_NUMBER_AVAILABLE
#       endif
#       if (_BYTE_ORDER == _PDP_ENDIAN)
#           undef POTHOS_ENDIAN_LITTLE_WORD
#           define POTHOS_ENDIAN_LITTLE_WORD POTHOS_VERSION_NUMBER_AVAILABLE
#       endif
#   endif
#endif

/* Built-in byte-swpped big-endian macros.
 */
#if !POTHOS_ENDIAN_BIG_BYTE && !POTHOS_ENDIAN_BIG_WORD && \
    !POTHOS_ENDIAN_LITTLE_BYTE && !POTHOS_ENDIAN_LITTLE_WORD
#   if !POTHOS_ENDIAN_BIG_BYTE
#       if (defined(__BIG_ENDIAN__) && !defined(__LITTLE_ENDIAN__)) || \
            defined(__ARMEB__) || \
            defined(__THUMBEB__) || \
            defined(__AARCH64EB__) || \
            defined(_MIPSEB) || \
            defined(__MIPSEB) || \
            defined(__MIPSEB__)
#           undef POTHOS_ENDIAN_BIG_BYTE
#           define POTHOS_ENDIAN_BIG_BYTE POTHOS_VERSION_NUMBER_AVAILABLE
#       endif
#   endif
#endif

/* Built-in byte-swpped little-endian macros.
 */
#if !POTHOS_ENDIAN_BIG_BYTE && !POTHOS_ENDIAN_BIG_WORD && \
    !POTHOS_ENDIAN_LITTLE_BYTE && !POTHOS_ENDIAN_LITTLE_WORD
#   if !POTHOS_ENDIAN_LITTLE_BYTE
#       if (defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__)) || \
            defined(__ARMEL__) || \
            defined(__THUMBEL__) || \
            defined(__AARCH64EL__) || \
            defined(_MIPSEL) || \
            defined(__MIPSEL) || \
            defined(__MIPSEL__)
#           undef POTHOS_ENDIAN_LITTLE_BYTE
#           define POTHOS_ENDIAN_LITTLE_BYTE POTHOS_VERSION_NUMBER_AVAILABLE
#       endif
#   endif
#endif

/* Some architectures are strictly one endianess (as opposed
 * the current common bi-endianess).
 */
#if !POTHOS_ENDIAN_BIG_BYTE && !POTHOS_ENDIAN_BIG_WORD && \
    !POTHOS_ENDIAN_LITTLE_BYTE && !POTHOS_ENDIAN_LITTLE_WORD
#   include <Pothos/serialization/impl/predef/architecture.h>
#   if POTHOS_ARCH_M68K || \
        POTHOS_ARCH_PARISK || \
        POTHOS_ARCH_SYS370 || \
        POTHOS_ARCH_SYS390 || \
        POTHOS_ARCH_Z
#       undef POTHOS_ENDIAN_BIG_BYTE
#       define POTHOS_ENDIAN_BIG_BYTE POTHOS_VERSION_NUMBER_AVAILABLE
#   endif
#   if POTHOS_ARCH_AMD64 || \
        POTHOS_ARCH_IA64 || \
        POTHOS_ARCH_X86 || \
        POTHOS_ARCH_BLACKFIN
#       undef POTHOS_ENDIAN_LITTLE_BYTE
#       define POTHOS_ENDIAN_LITTLE_BYTE POTHOS_VERSION_NUMBER_AVAILABLE
#   endif
#endif

/* Windows on ARM, if not otherwise detected/specified, is always
 * byte-swaped little-endian.
 */
#if !POTHOS_ENDIAN_BIG_BYTE && !POTHOS_ENDIAN_BIG_WORD && \
    !POTHOS_ENDIAN_LITTLE_BYTE && !POTHOS_ENDIAN_LITTLE_WORD
#   if POTHOS_ARCH_ARM
#       include <Pothos/serialization/impl/predef/os/windows.h>
#       if POTHOS_OS_WINDOWS
#           undef POTHOS_ENDIAN_LITTLE_BYTE
#           define POTHOS_ENDIAN_LITTLE_BYTE POTHOS_VERSION_NUMBER_AVAILABLE
#       endif
#   endif
#endif

#if POTHOS_ENDIAN_BIG_BYTE
#   define POTHOS_ENDIAN_BIG_BYTE_AVAILABLE
#endif
#if POTHOS_ENDIAN_BIG_WORD
#   define POTHOS_ENDIAN_BIG_WORD_BYTE_AVAILABLE
#endif
#if POTHOS_ENDIAN_LITTLE_BYTE
#   define POTHOS_ENDIAN_LITTLE_BYTE_AVAILABLE
#endif
#if POTHOS_ENDIAN_LITTLE_WORD
#   define POTHOS_ENDIAN_LITTLE_WORD_BYTE_AVAILABLE
#endif

#define POTHOS_ENDIAN_BIG_BYTE_NAME "Byte-Swapped Big-Endian"
#define POTHOS_ENDIAN_BIG_WORD_NAME "Word-Swapped Big-Endian"
#define POTHOS_ENDIAN_LITTLE_BYTE_NAME "Byte-Swapped Little-Endian"
#define POTHOS_ENDIAN_LITTLE_WORD_NAME "Word-Swapped Little-Endian"

#include <Pothos/serialization/impl/predef/detail/test.h>
POTHOS_PREDEF_DECLARE_TEST(POTHOS_ENDIAN_BIG_BYTE,POTHOS_ENDIAN_BIG_BYTE_NAME)

#include <Pothos/serialization/impl/predef/detail/test.h>
POTHOS_PREDEF_DECLARE_TEST(POTHOS_ENDIAN_BIG_WORD,POTHOS_ENDIAN_BIG_WORD_NAME)

#include <Pothos/serialization/impl/predef/detail/test.h>
POTHOS_PREDEF_DECLARE_TEST(POTHOS_ENDIAN_LITTLE_BYTE,POTHOS_ENDIAN_LITTLE_BYTE_NAME)

#include <Pothos/serialization/impl/predef/detail/test.h>
POTHOS_PREDEF_DECLARE_TEST(POTHOS_ENDIAN_LITTLE_WORD,POTHOS_ENDIAN_LITTLE_WORD_NAME)


#endif
