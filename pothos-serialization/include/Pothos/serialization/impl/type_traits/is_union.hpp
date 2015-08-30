
//  (C) Copyright Dave Abrahams, Steve Cleary, Beman Dawes, Howard
//  Hinnant & John Maddock 2000.  
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.


#ifndef POTHOS_TT_IS_UNION_HPP_INCLUDED
#define POTHOS_TT_IS_UNION_HPP_INCLUDED

#include <Pothos/serialization/impl/type_traits/remove_cv.hpp>
#include <Pothos/serialization/impl/type_traits/config.hpp>
#include <Pothos/serialization/impl/type_traits/intrinsics.hpp>

// should be the last #include
#include <Pothos/serialization/impl/type_traits/detail/bool_trait_def.hpp>

namespace Pothos {

namespace detail {
#ifndef __GNUC__
template <typename T> struct is_union_impl
{
   typedef typename remove_cv<T>::type cvt;
#ifdef POTHOS_IS_UNION
   POTHOS_STATIC_CONSTANT(bool, value = POTHOS_IS_UNION(cvt));
#else
   POTHOS_STATIC_CONSTANT(bool, value = false);
#endif
};
#else
//
// using remove_cv here generates a whole load of needless
// warnings with gcc, since it doesn't do any good with gcc
// in any case (at least at present), just remove it:
//
template <typename T> struct is_union_impl
{
#ifdef POTHOS_IS_UNION
   POTHOS_STATIC_CONSTANT(bool, value = POTHOS_IS_UNION(T));
#else
   POTHOS_STATIC_CONSTANT(bool, value = false);
#endif
};
#endif
} // namespace detail

POTHOS_TT_AUX_BOOL_TRAIT_DEF1(is_union,T,::Pothos::detail::is_union_impl<T>::value)

} // namespace boost

#include <Pothos/serialization/impl/type_traits/detail/bool_trait_undef.hpp>

#endif // BOOST_TT_IS_UNION_HPP_INCLUDED
