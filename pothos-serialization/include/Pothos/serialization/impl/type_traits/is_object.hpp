
//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef POTHOS_TT_IS_OBJECT_HPP_INCLUDED
#define POTHOS_TT_IS_OBJECT_HPP_INCLUDED

#include <Pothos/serialization/impl/type_traits/is_reference.hpp>
#include <Pothos/serialization/impl/type_traits/is_void.hpp>
#include <Pothos/serialization/impl/type_traits/is_function.hpp>
#include <Pothos/serialization/impl/type_traits/detail/ice_and.hpp>
#include <Pothos/serialization/impl/type_traits/detail/ice_not.hpp>
#include <Pothos/serialization/impl/config.hpp>

// should be the last #include
#include <Pothos/serialization/impl/type_traits/detail/bool_trait_def.hpp>

namespace Pothos {

namespace detail {

template <typename T>
struct is_object_impl
{
#ifndef POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION
   POTHOS_STATIC_CONSTANT(bool, value =
      (::Pothos::type_traits::ice_and<
         ::Pothos::type_traits::ice_not< ::Pothos::is_reference<T>::value>::value,
         ::Pothos::type_traits::ice_not< ::Pothos::is_void<T>::value>::value,
         ::Pothos::type_traits::ice_not< ::Pothos::is_function<T>::value>::value
      >::value));
#else
   POTHOS_STATIC_CONSTANT(bool, value =
      (::Pothos::type_traits::ice_and<
         ::Pothos::type_traits::ice_not< ::Pothos::is_reference<T>::value>::value,
         ::Pothos::type_traits::ice_not< ::Pothos::is_void<T>::value>::value
      >::value));
#endif
};

} // namespace detail

POTHOS_TT_AUX_BOOL_TRAIT_DEF1(is_object,T,::Pothos::detail::is_object_impl<T>::value)

} // namespace boost

#include <Pothos/serialization/impl/type_traits/detail/bool_trait_undef.hpp>

#endif // BOOST_TT_IS_OBJECT_HPP_INCLUDED
