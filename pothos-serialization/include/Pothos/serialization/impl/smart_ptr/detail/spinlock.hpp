#ifndef POTHOS_SMART_PTR_DETAIL_SPINLOCK_HPP_INCLUDED
#define POTHOS_SMART_PTR_DETAIL_SPINLOCK_HPP_INCLUDED

// MS compatible compilers support #pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

//
//  boost/detail/spinlock.hpp
//
//  Copyright (c) 2008 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
//  struct spinlock
//  {
//      void lock();
//      bool try_lock();
//      void unlock();
//
//      class scoped_lock;
//  };
//
//  #define BOOST_DETAIL_SPINLOCK_INIT <unspecified>
//

#include <Pothos/serialization/impl/config.hpp>
#include <Pothos/serialization/impl/smart_ptr/detail/sp_has_sync.hpp>

#if defined( POTHOS_SP_USE_PTHREADS )
#  include <Pothos/serialization/impl/smart_ptr/detail/spinlock_pt.hpp>

#elif defined(__GNUC__) && defined( __arm__ ) && !defined( __thumb__ )
#  include <Pothos/serialization/impl/smart_ptr/detail/spinlock_gcc_arm.hpp>

#elif defined( POTHOS_SP_HAS_SYNC )
#  include <Pothos/serialization/impl/smart_ptr/detail/spinlock_sync.hpp>

#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN__)
#  include <Pothos/serialization/impl/smart_ptr/detail/spinlock_w32.hpp>

#elif defined(POTHOS_HAS_PTHREADS)
#  include <Pothos/serialization/impl/smart_ptr/detail/spinlock_pt.hpp>

#elif !defined(POTHOS_HAS_THREADS)
#  include <Pothos/serialization/impl/smart_ptr/detail/spinlock_nt.hpp>

#else
#  error Unrecognized threading platform
#endif

#endif // #ifndef BOOST_SMART_PTR_DETAIL_SPINLOCK_HPP_INCLUDED
