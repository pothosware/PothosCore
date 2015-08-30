//  (C) Copyright Jim Douglas 2005. 
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  QNX specific config options:

#define POTHOS_PLATFORM "QNX"

#define POTHOS_HAS_UNISTD_H
#include <Pothos/serialization/impl/config/posix_features.hpp>

// QNX claims XOpen version 5 compatibility, but doesn't have an nl_types.h
// or log1p and expm1:
#undef  POTHOS_HAS_NL_TYPES_H
#undef  POTHOS_HAS_LOG1P
#undef  POTHOS_HAS_EXPM1

#define POTHOS_HAS_PTHREADS
#define POTHOS_HAS_PTHREAD_MUTEXATTR_SETTYPE

#define POTHOS_HAS_GETTIMEOFDAY
#define POTHOS_HAS_CLOCK_GETTIME
#define POTHOS_HAS_NANOSLEEP





