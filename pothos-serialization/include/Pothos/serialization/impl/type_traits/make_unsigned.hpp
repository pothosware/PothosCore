
//  (C) Copyright John Maddock 2007.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef POTHOS_TT_MAKE_UNSIGNED_HPP_INCLUDED
#define POTHOS_TT_MAKE_UNSIGNED_HPP_INCLUDED

#include <Pothos/serialization/impl/mpl/if.hpp>
#include <Pothos/serialization/impl/type_traits/is_integral.hpp>
#include <Pothos/serialization/impl/type_traits/is_signed.hpp>
#include <Pothos/serialization/impl/type_traits/is_unsigned.hpp>
#include <Pothos/serialization/impl/type_traits/is_enum.hpp>
#include <Pothos/serialization/impl/type_traits/is_same.hpp>
#include <Pothos/serialization/impl/type_traits/remove_cv.hpp>
#include <Pothos/serialization/impl/type_traits/is_const.hpp>
#include <Pothos/serialization/impl/type_traits/is_volatile.hpp>
#include <Pothos/serialization/impl/type_traits/add_const.hpp>
#include <Pothos/serialization/impl/type_traits/add_volatile.hpp>
#include <Pothos/serialization/impl/type_traits/detail/ice_or.hpp>
#include <Pothos/serialization/impl/type_traits/detail/ice_and.hpp>
#include <Pothos/serialization/impl/type_traits/detail/ice_not.hpp>
#include <Pothos/serialization/impl/static_assert.hpp>

// should be the last #include
#include <Pothos/serialization/impl/type_traits/detail/type_trait_def.hpp>

namespace Pothos {

namespace detail {

template <class T>
struct make_unsigned_imp
{
   POTHOS_STATIC_ASSERT(
      (::Pothos::type_traits::ice_or< ::Pothos::is_integral<T>::value, ::Pothos::is_enum<T>::value>::value));
#if !POTHOS_WORKAROUND(POTHOS_MSVC, <=1300)
   POTHOS_STATIC_ASSERT(
      (::Pothos::type_traits::ice_not< ::Pothos::is_same<
         typename remove_cv<T>::type, bool>::value>::value));
#endif

   typedef typename remove_cv<T>::type t_no_cv;
   typedef typename mpl::if_c<
      (::Pothos::type_traits::ice_and< 
         ::Pothos::is_unsigned<T>::value,
         ::Pothos::is_integral<T>::value,
         ::Pothos::type_traits::ice_not< ::Pothos::is_same<t_no_cv, char>::value>::value,
         ::Pothos::type_traits::ice_not< ::Pothos::is_same<t_no_cv, wchar_t>::value>::value,
         ::Pothos::type_traits::ice_not< ::Pothos::is_same<t_no_cv, bool>::value>::value >::value),
      T,
      typename mpl::if_c<
         (::Pothos::type_traits::ice_and< 
            ::Pothos::is_integral<T>::value,
            ::Pothos::type_traits::ice_not< ::Pothos::is_same<t_no_cv, char>::value>::value,
            ::Pothos::type_traits::ice_not< ::Pothos::is_same<t_no_cv, wchar_t>::value>::value,
            ::Pothos::type_traits::ice_not< ::Pothos::is_same<t_no_cv, bool>::value>::value>
         ::value),
         typename mpl::if_<
            is_same<t_no_cv, signed char>,
            unsigned char,
            typename mpl::if_<
               is_same<t_no_cv, short>,
               unsigned short,
               typename mpl::if_<
                  is_same<t_no_cv, int>,
                  unsigned int,
                  typename mpl::if_<
                     is_same<t_no_cv, long>,
                     unsigned long,
#if defined(POTHOS_HAS_LONG_LONG)
#ifdef POTHOS_HAS_INT128
                     typename mpl::if_c<
                        sizeof(t_no_cv) == sizeof(Pothos::ulong_long_type), 
                        Pothos::ulong_long_type, 
                        Pothos::uint128_type
                     >::type
#else
                     Pothos::ulong_long_type
#endif
#elif defined(POTHOS_HAS_MS_INT64)
                     unsigned __int64
#else
                     unsigned long
#endif
                  >::type
               >::type
            >::type
         >::type,
         // Not a regular integer type:
         typename mpl::if_c<
            sizeof(t_no_cv) == sizeof(unsigned char),
            unsigned char,
            typename mpl::if_c<
               sizeof(t_no_cv) == sizeof(unsigned short),
               unsigned short,
               typename mpl::if_c<
                  sizeof(t_no_cv) == sizeof(unsigned int),
                  unsigned int,
                  typename mpl::if_c<
                     sizeof(t_no_cv) == sizeof(unsigned long),
                     unsigned long,
#if defined(POTHOS_HAS_LONG_LONG)
#ifdef POTHOS_HAS_INT128
                     typename mpl::if_c<
                        sizeof(t_no_cv) == sizeof(Pothos::ulong_long_type), 
                        Pothos::ulong_long_type, 
                        Pothos::uint128_type
                     >::type
#else
                     Pothos::ulong_long_type
#endif
#elif defined(POTHOS_HAS_MS_INT64)
                     unsigned __int64
#else
                     unsigned long
#endif
                  >::type
               >::type
            >::type
         >::type
      >::type
   >::type base_integer_type;
   
   // Add back any const qualifier:
   typedef typename mpl::if_<
      is_const<T>,
      typename add_const<base_integer_type>::type,
      base_integer_type
   >::type const_base_integer_type;
   
   // Add back any volatile qualifier:
   typedef typename mpl::if_<
      is_volatile<T>,
      typename add_volatile<const_base_integer_type>::type,
      const_base_integer_type
   >::type type;
};


} // namespace detail

POTHOS_TT_AUX_TYPE_TRAIT_DEF1(make_unsigned,T,typename Pothos::detail::make_unsigned_imp<T>::type)

} // namespace boost

#include <Pothos/serialization/impl/type_traits/detail/type_trait_undef.hpp>

#endif // BOOST_TT_ADD_REFERENCE_HPP_INCLUDED

