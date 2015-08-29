
//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef POTHOS_TT_HAS_NOTHROW_CONSTRUCTOR_HPP_INCLUDED
#define POTHOS_TT_HAS_NOTHROW_CONSTRUCTOR_HPP_INCLUDED

#include <Pothos/serialization/impl/type_traits/has_trivial_constructor.hpp>

// should be the last #include
#include <Pothos/serialization/impl/type_traits/detail/bool_trait_def.hpp>

namespace Pothos {

namespace detail{

template <class T>
struct has_nothrow_constructor_imp{
#ifdef POTHOS_HAS_NOTHROW_CONSTRUCTOR
   POTHOS_STATIC_CONSTANT(bool, value = POTHOS_HAS_NOTHROW_CONSTRUCTOR(T));
#else
   POTHOS_STATIC_CONSTANT(bool, value = ::Pothos::has_trivial_constructor<T>::value);
#endif
};

}

POTHOS_TT_AUX_BOOL_TRAIT_DEF1(has_nothrow_constructor,T,::Pothos::detail::has_nothrow_constructor_imp<T>::value)
POTHOS_TT_AUX_BOOL_TRAIT_DEF1(has_nothrow_default_constructor,T,::Pothos::detail::has_nothrow_constructor_imp<T>::value)

POTHOS_TT_AUX_BOOL_TRAIT_SPEC1(has_nothrow_constructor,void,false)
#ifndef POTHOS_NO_CV_VOID_SPECIALIZATIONS
POTHOS_TT_AUX_BOOL_TRAIT_SPEC1(has_nothrow_constructor,void const,false)
POTHOS_TT_AUX_BOOL_TRAIT_SPEC1(has_nothrow_constructor,void const volatile,false)
POTHOS_TT_AUX_BOOL_TRAIT_SPEC1(has_nothrow_constructor,void volatile,false)
#endif

POTHOS_TT_AUX_BOOL_TRAIT_SPEC1(has_nothrow_default_constructor,void,false)
#ifndef POTHOS_NO_CV_VOID_SPECIALIZATIONS
POTHOS_TT_AUX_BOOL_TRAIT_SPEC1(has_nothrow_default_constructor,void const,false)
POTHOS_TT_AUX_BOOL_TRAIT_SPEC1(has_nothrow_default_constructor,void const volatile,false)
POTHOS_TT_AUX_BOOL_TRAIT_SPEC1(has_nothrow_default_constructor,void volatile,false)
#endif

} // namespace boost

#include <Pothos/serialization/impl/type_traits/detail/bool_trait_undef.hpp>

#endif // BOOST_TT_HAS_NOTHROW_CONSTRUCTOR_HPP_INCLUDED
