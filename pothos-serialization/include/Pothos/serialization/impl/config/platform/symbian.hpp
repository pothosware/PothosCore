//  (C) Copyright Yuriy Krasnoschek 2009. 
//  (C) Copyright John Maddock 2001 - 2003. 
//  (C) Copyright Jens Maurer 2001 - 2003. 
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  symbian specific config options:


#define POTHOS_PLATFORM "Symbian"
#define POTHOS_SYMBIAN 1


#if defined(__S60_3X__)
// Open C / C++ plugin was introdused in this SDK, earlier versions don't have CRT / STL
#  define POTHOS_S60_3rd_EDITION_FP2_OR_LATER_SDK
// make sure we have __GLIBC_PREREQ if available at all
#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif// boilerplate code:
#  define POTHOS_HAS_UNISTD_H
#  include <Pothos/serialization/impl/config/posix_features.hpp>
// S60 SDK defines _POSIX_VERSION as POSIX.1
#  ifndef POTHOS_HAS_STDINT_H
#    define POTHOS_HAS_STDINT_H
#  endif
#  ifndef POTHOS_HAS_GETTIMEOFDAY
#    define POTHOS_HAS_GETTIMEOFDAY
#  endif
#  ifndef POTHOS_HAS_DIRENT_H
#    define POTHOS_HAS_DIRENT_H
#  endif
#  ifndef POTHOS_HAS_SIGACTION
#    define POTHOS_HAS_SIGACTION
#  endif
#  ifndef POTHOS_HAS_PTHREADS
#    define POTHOS_HAS_PTHREADS
#  endif
#  ifndef POTHOS_HAS_NANOSLEEP
#    define POTHOS_HAS_NANOSLEEP
#  endif
#  ifndef POTHOS_HAS_SCHED_YIELD
#    define POTHOS_HAS_SCHED_YIELD
#  endif
#  ifndef POTHOS_HAS_PTHREAD_MUTEXATTR_SETTYPE
#    define POTHOS_HAS_PTHREAD_MUTEXATTR_SETTYPE
#  endif
#  ifndef POTHOS_HAS_LOG1P
#    define POTHOS_HAS_LOG1P
#  endif
#  ifndef POTHOS_HAS_EXPM1
#    define POTHOS_HAS_EXPM1
#  endif
#  ifndef POTHOS_POSIX_API
#    define POTHOS_POSIX_API
#  endif
// endianess support
#  include <sys/endian.h>
// Symbian SDK provides _BYTE_ORDER instead of __BYTE_ORDER
#  ifndef __LITTLE_ENDIAN
#    ifdef _LITTLE_ENDIAN
#      define __LITTLE_ENDIAN _LITTLE_ENDIAN
#    else
#      define __LITTLE_ENDIAN 1234
#    endif
#  endif
#  ifndef __BIG_ENDIAN
#    ifdef _BIG_ENDIAN
#      define __BIG_ENDIAN _BIG_ENDIAN
#    else
#      define __BIG_ENDIAN 4321
#    endif
#  endif
#  ifndef __BYTE_ORDER
#    define __BYTE_ORDER __LITTLE_ENDIAN // Symbian is LE
#  endif
// Known limitations
#  define POTHOS_ASIO_DISABLE_SERIAL_PORT
#  define POTHOS_DATE_TIME_NO_LOCALE
#  define POTHOS_NO_STD_WSTRING
#  define POTHOS_EXCEPTION_DISABLE
#  define POTHOS_NO_EXCEPTIONS

#else // TODO: More platform support e.g. UIQ
#  error "Unsuppoted Symbian SDK"
#endif

#if defined(__WINSCW__) && !defined(POTHOS_DISABLE_WIN32)
#  define POTHOS_DISABLE_WIN32 // winscw defines WIN32 macro
#endif


