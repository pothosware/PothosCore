
//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef POTHOS_TT_HAS_TRIVIAL_CONSTRUCTOR_HPP_INCLUDED
#define POTHOS_TT_HAS_TRIVIAL_CONSTRUCTOR_HPP_INCLUDED

#include <Pothos/serialization/impl/type_traits/config.hpp>
#include <Pothos/serialization/impl/type_traits/intrinsics.hpp>
#include <Pothos/serialization/impl/type_traits/is_pod.hpp>
#include <Pothos/serialization/impl/type_traits/detail/ice_or.hpp>

// should be the last #include
#include <Pothos/serialization/impl/type_traits/detail/bool_trait_def.hpp>

namespace Pothos {

namespace detail {

template <typename T>
struct has_trivial_ctor_impl
{
#ifdef POTHOS_HAS_TRIVIAL_CONSTRUCTOR
   POTHOS_STATIC_CONSTANT(bool, value =
      (::Pothos::type_traits::ice_or<
         ::Pothos::is_pod<T>::value,
         POTHOS_HAS_TRIVIAL_CONSTRUCTOR(T)
      >::value));
#else
   POTHOS_STATIC_CONSTANT(bool, value =
      (::Pothos::type_traits::ice_or<
         ::Pothos::is_pod<T>::value,
         false
      >::value));
#endif
};

} // namespace detail

POTHOS_TT_AUX_BOOL_TRAIT_DEF1(has_trivial_constructor,T,::Pothos::detail::has_trivial_ctor_impl<T>::value)
POTHOS_TT_AUX_BOOL_TRAIT_DEF1(has_trivial_default_constructor,T,::Pothos::detail::has_trivial_ctor_impl<T>::value)

} // namespace boost

#include <Pothos/serialization/impl/type_traits/detail/bool_trait_undef.hpp>

#endif // BOOST_TT_HAS_TRIVIAL_CONSTRUCTOR_HPP_INCLUDED
