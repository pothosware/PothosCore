//  (C) Copyright John Maddock 2001. 
//  (C) Copyright Darin Adler 2001. 
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  Metrowerks standard library:

#ifndef __MSL_CPP__
#  include <Pothos/serialization/impl/config/no_tr1/utility.hpp>
#  ifndef __MSL_CPP__
#     error This is not the MSL standard library!
#  endif
#endif

#if __MSL_CPP__ >= 0x6000  // Pro 6
#  define POTHOS_HAS_HASH
#  define POTHOS_STD_EXTENSION_NAMESPACE Metrowerks
#endif
#define POTHOS_HAS_SLIST

#if __MSL_CPP__ < 0x6209
#  define POTHOS_NO_STD_MESSAGES
#endif

// check C lib version for <stdint.h>
#include <cstddef>

#if defined(__MSL__) && (__MSL__ >= 0x5000)
#  define POTHOS_HAS_STDINT_H
#  if !defined(__PALMOS_TRAPS__)
#    define POTHOS_HAS_UNISTD_H
#  endif
   // boilerplate code:
#  include <Pothos/serialization/impl/config/posix_features.hpp>
#endif

#if defined(_MWMT) || _MSL_THREADSAFE
#  define POTHOS_HAS_THREADS
#endif

#ifdef _MSL_NO_EXPLICIT_FUNC_TEMPLATE_ARG
#  define POTHOS_NO_STD_USE_FACET
#  define POTHOS_HAS_TWO_ARG_USE_FACET
#endif

//  C++0x headers not yet implemented
//
#  define POTHOS_NO_CXX11_HDR_ARRAY
#  define POTHOS_NO_CXX11_HDR_CHRONO
#  define POTHOS_NO_CXX11_HDR_CODECVT
#  define POTHOS_NO_CXX11_HDR_CONDITION_VARIABLE
#  define POTHOS_NO_CXX11_HDR_FORWARD_LIST
#  define POTHOS_NO_CXX11_HDR_FUTURE
#  define POTHOS_NO_CXX11_HDR_INITIALIZER_LIST
#  define POTHOS_NO_CXX11_HDR_MUTEX
#  define POTHOS_NO_CXX11_HDR_RANDOM
#  define POTHOS_NO_CXX11_HDR_RATIO
#  define POTHOS_NO_CXX11_HDR_REGEX
#  define POTHOS_NO_CXX11_HDR_SYSTEM_ERROR
#  define POTHOS_NO_CXX11_HDR_THREAD
#  define POTHOS_NO_CXX11_HDR_TUPLE
#  define POTHOS_NO_CXX11_HDR_TYPE_TRAITS
#  define POTHOS_NO_CXX11_HDR_TYPEINDEX
#  define POTHOS_NO_CXX11_HDR_UNORDERED_MAP
#  define POTHOS_NO_CXX11_HDR_UNORDERED_SET
#  define POTHOS_NO_CXX11_NUMERIC_LIMITS
#  define POTHOS_NO_CXX11_ALLOCATOR
#  define POTHOS_NO_CXX11_ATOMIC_SMART_PTR
#  define POTHOS_NO_CXX11_SMART_PTR
#  define POTHOS_NO_CXX11_HDR_FUNCTIONAL

#define POTHOS_STDLIB "Metrowerks Standard Library version " POTHOS_STRINGIZE(__MSL_CPP__)









