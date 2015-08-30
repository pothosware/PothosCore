#ifndef POTHOS_SMART_PTR_DETAIL_SP_NULLPTR_T_HPP_INCLUDED
#define POTHOS_SMART_PTR_DETAIL_SP_NULLPTR_T_HPP_INCLUDED

// MS compatible compilers support #pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

//  detail/sp_nullptr_t.hpp
//
//  Copyright 2013 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt

#include <Pothos/serialization/impl/config.hpp>
#include <cstddef>

#if !defined( POTHOS_NO_CXX11_NULLPTR )

namespace Pothos
{

namespace detail
{

#if defined( __clang__ ) && !defined( _LIBCPP_VERSION ) && !defined( POTHOS_NO_CXX11_DECLTYPE )

    typedef decltype(nullptr) sp_nullptr_t;

#else

    typedef std::nullptr_t sp_nullptr_t;

#endif

} // namespace detail

} // namespace boost

#endif // !defined( BOOST_NO_CXX11_NULLPTR )

#endif  // #ifndef BOOST_SMART_PTR_DETAIL_SP_NULLPTR_T_HPP_INCLUDED
