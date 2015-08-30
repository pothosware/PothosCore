
//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef POTHOS_TT_HAS_TRIVIAL_DESTRUCTOR_HPP_INCLUDED
#define POTHOS_TT_HAS_TRIVIAL_DESTRUCTOR_HPP_INCLUDED

#include <Pothos/serialization/impl/type_traits/config.hpp>
#include <Pothos/serialization/impl/type_traits/intrinsics.hpp>
#include <Pothos/serialization/impl/type_traits/is_pod.hpp>
#include <Pothos/serialization/impl/type_traits/detail/ice_or.hpp>

// should be the last #include
#include <Pothos/serialization/impl/type_traits/detail/bool_trait_def.hpp>

namespace Pothos {

namespace detail {

template <typename T>
struct has_trivial_dtor_impl
{
#ifdef POTHOS_HAS_TRIVIAL_DESTRUCTOR
   POTHOS_STATIC_CONSTANT(bool, value = POTHOS_HAS_TRIVIAL_DESTRUCTOR(T));
#else
   POTHOS_STATIC_CONSTANT(bool, value = ::Pothos::is_pod<T>::value);
#endif
};

} // namespace detail

POTHOS_TT_AUX_BOOL_TRAIT_DEF1(has_trivial_destructor,T,::Pothos::detail::has_trivial_dtor_impl<T>::value)

POTHOS_TT_AUX_BOOL_TRAIT_SPEC1(has_trivial_destructor,void,false)
#ifndef POTHOS_NO_CV_VOID_SPECIALIZATIONS
POTHOS_TT_AUX_BOOL_TRAIT_SPEC1(has_trivial_destructor,void const,false)
POTHOS_TT_AUX_BOOL_TRAIT_SPEC1(has_trivial_destructor,void const volatile,false)
POTHOS_TT_AUX_BOOL_TRAIT_SPEC1(has_trivial_destructor,void volatile,false)
#endif

} // namespace boost

#include <Pothos/serialization/impl/type_traits/detail/bool_trait_undef.hpp>

#endif // BOOST_TT_HAS_TRIVIAL_DESTRUCTOR_HPP_INCLUDED
