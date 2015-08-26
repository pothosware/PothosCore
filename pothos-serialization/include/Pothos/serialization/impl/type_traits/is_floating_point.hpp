
//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000-2005.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef POTHOS_TYPE_TRAITS_IS_FLOATING_HPP_INCLUDED
#define POTHOS_TYPE_TRAITS_IS_FLOATING_HPP_INCLUDED

// should be the last #include
#include <Pothos/serialization/impl/type_traits/detail/bool_trait_def.hpp>

namespace Pothos {

//* is a type T a floating-point type described in the standard (3.9.1p8)
POTHOS_TT_AUX_BOOL_TRAIT_DEF1(is_floating_point,T,false)
POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_floating_point,float,true)
POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_floating_point,double,true)
POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_floating_point,long double,true)

} // namespace boost

#include <Pothos/serialization/impl/type_traits/detail/bool_trait_undef.hpp>

#endif // BOOST_TYPE_TRAITS_IS_FLOAT_HPP_INCLUDED
