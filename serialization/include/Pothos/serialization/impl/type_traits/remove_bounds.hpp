
//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef POTHOS_TT_REMOVE_BOUNDS_HPP_INCLUDED
#define POTHOS_TT_REMOVE_BOUNDS_HPP_INCLUDED

#include <Pothos/serialization/impl/config.hpp>
#include <cstddef>
#include <Pothos/serialization/impl/detail/workaround.hpp>

#if POTHOS_WORKAROUND(POTHOS_MSVC,<=1300)
#include <Pothos/serialization/impl/type_traits/msvc/remove_bounds.hpp>
#endif

// should be the last #include
#include <Pothos/serialization/impl/type_traits/detail/type_trait_def.hpp>

#if !POTHOS_WORKAROUND(POTHOS_MSVC,<=1300)

namespace Pothos {

POTHOS_TT_AUX_TYPE_TRAIT_DEF1(remove_bounds,T,T)

#if !defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION) && !defined(POTHOS_NO_ARRAY_TYPE_SPECIALIZATIONS)
POTHOS_TT_AUX_TYPE_TRAIT_PARTIAL_SPEC1_2(typename T,std::size_t N,remove_bounds,T[N],T type)
POTHOS_TT_AUX_TYPE_TRAIT_PARTIAL_SPEC1_2(typename T,std::size_t N,remove_bounds,T const[N],T const type)
POTHOS_TT_AUX_TYPE_TRAIT_PARTIAL_SPEC1_2(typename T,std::size_t N,remove_bounds,T volatile[N],T volatile type)
POTHOS_TT_AUX_TYPE_TRAIT_PARTIAL_SPEC1_2(typename T,std::size_t N,remove_bounds,T const volatile[N],T const volatile type)
#if !POTHOS_WORKAROUND(__BORLANDC__, POTHOS_TESTED_AT(0x610)) && !defined(__IBMCPP__) &&  !POTHOS_WORKAROUND(__DMC__, POTHOS_TESTED_AT(0x840))
POTHOS_TT_AUX_TYPE_TRAIT_PARTIAL_SPEC1_1(typename T,remove_bounds,T[],T)
POTHOS_TT_AUX_TYPE_TRAIT_PARTIAL_SPEC1_1(typename T,remove_bounds,T const[],T const)
POTHOS_TT_AUX_TYPE_TRAIT_PARTIAL_SPEC1_1(typename T,remove_bounds,T volatile[],T volatile)
POTHOS_TT_AUX_TYPE_TRAIT_PARTIAL_SPEC1_1(typename T,remove_bounds,T const volatile[],T const volatile)
#endif
#endif

} // namespace boost

#endif

#include <Pothos/serialization/impl/type_traits/detail/type_trait_undef.hpp>

#endif // BOOST_TT_REMOVE_BOUNDS_HPP_INCLUDED
