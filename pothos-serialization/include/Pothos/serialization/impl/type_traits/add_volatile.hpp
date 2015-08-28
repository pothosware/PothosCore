
//  (C) Copyright Dave Abrahams, Steve Cleary, Beman Dawes, Howard
//  Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef POTHOS_TT_ADD_VOLATILE_HPP_INCLUDED
#define POTHOS_TT_ADD_VOLATILE_HPP_INCLUDED

#include <Pothos/serialization/impl/config.hpp>

// should be the last #include
#include <Pothos/serialization/impl/type_traits/detail/type_trait_def.hpp>

namespace Pothos {

// * convert a type T to volatile type - add_volatile<T>
// this is not required since the result is always
// the same as "T volatile", but it does suppress warnings
// from some compilers:

#if defined(POTHOS_MSVC)
// This bogus warning will appear when add_volatile is applied to a
// const volatile reference because we can't detect const volatile
// references with MSVC6.
#   pragma warning(push)
#   pragma warning(disable:4181) // warning C4181: qualifier applied to reference type ignored
#endif 

POTHOS_TT_AUX_TYPE_TRAIT_DEF1(add_volatile,T,T volatile)

#if defined(POTHOS_MSVC)
#   pragma warning(pop)
#endif 

#ifndef POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION
POTHOS_TT_AUX_TYPE_TRAIT_PARTIAL_SPEC1_1(typename T,add_volatile,T&,T&)
#endif

} // namespace boost

#include <Pothos/serialization/impl/type_traits/detail/type_trait_undef.hpp>

#endif // BOOST_TT_ADD_VOLATILE_HPP_INCLUDED
