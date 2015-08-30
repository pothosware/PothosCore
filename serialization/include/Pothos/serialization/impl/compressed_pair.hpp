//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/utility for most recent version including documentation.

//  See boost/detail/compressed_pair.hpp and boost/detail/ob_compressed_pair.hpp
//  for full copyright notices.

#ifndef POTHOS_COMPRESSED_PAIR_HPP
#define POTHOS_COMPRESSED_PAIR_HPP

#ifndef POTHOS_CONFIG_HPP
#include <Pothos/serialization/impl/config.hpp>
#endif

#ifdef POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION
#include <Pothos/serialization/impl/detail/ob_compressed_pair.hpp>
#else
#include <Pothos/serialization/impl/detail/compressed_pair.hpp>
#endif

#endif // BOOST_COMPRESSED_PAIR_HPP
