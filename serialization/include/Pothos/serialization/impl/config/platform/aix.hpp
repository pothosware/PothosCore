//  (C) Copyright John Maddock 2001 - 2002. 
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  IBM/Aix specific config options:

#define POTHOS_PLATFORM "IBM Aix"

#define POTHOS_HAS_UNISTD_H
#define POTHOS_HAS_NL_TYPES_H
#define POTHOS_HAS_NANOSLEEP
#define POTHOS_HAS_CLOCK_GETTIME

// This needs support in "boost/cstdint.hpp" exactly like FreeBSD.
// This platform has header named <inttypes.h> which includes all
// the things needed.
#define POTHOS_HAS_STDINT_H

// Threading API's:
#define POTHOS_HAS_PTHREADS
#define POTHOS_HAS_PTHREAD_DELAY_NP
#define POTHOS_HAS_SCHED_YIELD
//#define BOOST_HAS_PTHREAD_YIELD

// boilerplate code:
#include <Pothos/serialization/impl/config/posix_features.hpp>




