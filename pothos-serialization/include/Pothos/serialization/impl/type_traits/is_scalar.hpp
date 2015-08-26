
//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef POTHOS_TT_IS_SCALAR_HPP_INCLUDED
#define POTHOS_TT_IS_SCALAR_HPP_INCLUDED

#include <Pothos/serialization/impl/type_traits/is_arithmetic.hpp>
#include <Pothos/serialization/impl/type_traits/is_enum.hpp>
#include <Pothos/serialization/impl/type_traits/is_pointer.hpp>
#include <Pothos/serialization/impl/type_traits/is_member_pointer.hpp>
#include <Pothos/serialization/impl/type_traits/detail/ice_or.hpp>
#include <Pothos/serialization/impl/config.hpp>

// should be the last #include
#include <Pothos/serialization/impl/type_traits/detail/bool_trait_def.hpp>

namespace Pothos {

namespace detail {

template <typename T>
struct is_scalar_impl
{ 
   POTHOS_STATIC_CONSTANT(bool, value =
      (::Pothos::type_traits::ice_or<
         ::Pothos::is_arithmetic<T>::value,
         ::Pothos::is_enum<T>::value,
         ::Pothos::is_pointer<T>::value,
         ::Pothos::is_member_pointer<T>::value
      >::value));
};

// these specializations are only really needed for compilers 
// without partial specialization support:
template <> struct is_scalar_impl<void>{ POTHOS_STATIC_CONSTANT(bool, value = false ); };
#ifndef POTHOS_NO_CV_VOID_SPECIALIZATIONS
template <> struct is_scalar_impl<void const>{ POTHOS_STATIC_CONSTANT(bool, value = false ); };
template <> struct is_scalar_impl<void volatile>{ POTHOS_STATIC_CONSTANT(bool, value = false ); };
template <> struct is_scalar_impl<void const volatile>{ POTHOS_STATIC_CONSTANT(bool, value = false ); };
#endif

} // namespace detail

POTHOS_TT_AUX_BOOL_TRAIT_DEF1(is_scalar,T,::Pothos::detail::is_scalar_impl<T>::value)

} // namespace boost

#include <Pothos/serialization/impl/type_traits/detail/bool_trait_undef.hpp>

#endif // BOOST_TT_IS_SCALAR_HPP_INCLUDED
