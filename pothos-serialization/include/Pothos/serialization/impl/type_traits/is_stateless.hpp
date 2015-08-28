
//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef POTHOS_TT_IS_STATELESS_HPP_INCLUDED
#define POTHOS_TT_IS_STATELESS_HPP_INCLUDED

#include <Pothos/serialization/impl/type_traits/has_trivial_constructor.hpp>
#include <Pothos/serialization/impl/type_traits/has_trivial_copy.hpp>
#include <Pothos/serialization/impl/type_traits/has_trivial_destructor.hpp>
#include <Pothos/serialization/impl/type_traits/is_class.hpp>
#include <Pothos/serialization/impl/type_traits/is_empty.hpp>
#include <Pothos/serialization/impl/type_traits/detail/ice_and.hpp>
#include <Pothos/serialization/impl/config.hpp>

// should be the last #include
#include <Pothos/serialization/impl/type_traits/detail/bool_trait_def.hpp>

namespace Pothos {

namespace detail {

template <typename T>
struct is_stateless_impl
{
  POTHOS_STATIC_CONSTANT(bool, value = 
    (::Pothos::type_traits::ice_and<
       ::Pothos::has_trivial_constructor<T>::value,
       ::Pothos::has_trivial_copy<T>::value,
       ::Pothos::has_trivial_destructor<T>::value,
       ::Pothos::is_class<T>::value,
       ::Pothos::is_empty<T>::value
     >::value));
};

} // namespace detail

POTHOS_TT_AUX_BOOL_TRAIT_DEF1(is_stateless,T,::Pothos::detail::is_stateless_impl<T>::value)

} // namespace boost

#include <Pothos/serialization/impl/type_traits/detail/bool_trait_undef.hpp>

#endif // BOOST_TT_IS_STATELESS_HPP_INCLUDED
