
//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef POTHOS_TT_IS_FUNDAMENTAL_HPP_INCLUDED
#define POTHOS_TT_IS_FUNDAMENTAL_HPP_INCLUDED

#include <Pothos/serialization/impl/type_traits/is_arithmetic.hpp>
#include <Pothos/serialization/impl/type_traits/is_void.hpp>
#include <Pothos/serialization/impl/type_traits/detail/ice_or.hpp>

// should be the last #include
#include <Pothos/serialization/impl/type_traits/detail/bool_trait_def.hpp>

namespace Pothos {

namespace detail {

template <typename T> 
struct is_fundamental_impl
    : public ::Pothos::type_traits::ice_or< 
          ::Pothos::is_arithmetic<T>::value
        , ::Pothos::is_void<T>::value
        >
{ 
};

} // namespace detail

//* is a type T a fundamental type described in the standard (3.9.1)
#if defined( __CODEGEARC__ )
POTHOS_TT_AUX_BOOL_TRAIT_DEF1(is_fundamental,T,__is_fundamental(T))
#else
POTHOS_TT_AUX_BOOL_TRAIT_DEF1(is_fundamental,T,::Pothos::detail::is_fundamental_impl<T>::value)
#endif

} // namespace boost

#include <Pothos/serialization/impl/type_traits/detail/bool_trait_undef.hpp>

#endif // BOOST_TT_IS_FUNDAMENTAL_HPP_INCLUDED
