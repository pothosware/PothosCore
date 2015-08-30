//  (C) Copyright Christopher Jefferson 2011.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  config for libc++
//  Might need more in here later.

#if !defined(_LIBCPP_VERSION)
#  include <ciso646>
#  if !defined(_LIBCPP_VERSION)
#      error "This is not libc++!"
#  endif
#endif

#define POTHOS_STDLIB "libc++ version " POTHOS_STRINGIZE(_LIBCPP_VERSION)

#define POTHOS_HAS_THREADS

#ifdef _LIBCPP_HAS_NO_VARIADICS
#    define POTHOS_NO_CXX11_HDR_TUPLE
#endif

//
// These appear to be unusable/incomplete so far:
//
#  define POTHOS_NO_CXX11_HDR_CHRONO
#  define POTHOS_NO_CXX11_HDR_FUTURE
#  define POTHOS_NO_CXX11_HDR_TYPE_TRAITS
#  define POTHOS_NO_CXX11_ATOMIC_SMART_PTR

// libc++ uses a non-standard messages_base
#define POTHOS_NO_STD_MESSAGES

//  --- end ---
