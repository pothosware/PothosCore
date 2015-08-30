//  (C) Copyright John Maddock 2001. 
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  BeOS specific config options:

#define POTHOS_PLATFORM "BeOS"

#define POTHOS_NO_CWCHAR
#define POTHOS_NO_CWCTYPE
#define POTHOS_HAS_UNISTD_H

#define POTHOS_HAS_BETHREADS

#ifndef POTHOS_DISABLE_THREADS
#  define POTHOS_HAS_THREADS
#endif

// boilerplate code:
#include <Pothos/serialization/impl/config/posix_features.hpp>
 


