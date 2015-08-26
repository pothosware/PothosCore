
//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef POTHOS_TT_REMOVE_POINTER_HPP_INCLUDED
#define POTHOS_TT_REMOVE_POINTER_HPP_INCLUDED

#include <Pothos/serialization/impl/config.hpp>
#include <Pothos/serialization/impl/detail/workaround.hpp>
#ifdef POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION
#include <Pothos/serialization/impl/type_traits/broken_compiler_spec.hpp>
#endif

#if POTHOS_WORKAROUND(POTHOS_MSVC,<=1300)
#include <Pothos/serialization/impl/type_traits/msvc/remove_pointer.hpp>
#elif defined(POTHOS_MSVC)
#include <Pothos/serialization/impl/type_traits/remove_cv.hpp>
#include <Pothos/serialization/impl/type_traits/is_pointer.hpp>
#endif

// should be the last #include
#include <Pothos/serialization/impl/type_traits/detail/type_trait_def.hpp>

namespace Pothos {

#ifdef POTHOS_MSVC

namespace detail{

   //
   // We need all this crazy indirection because a type such as:
   //
   // T (*const)(U)
   //
   // Does not bind to a <T*> or <T*const> partial specialization with VC10 and earlier
   //
   template <class T> 
   struct remove_pointer_imp
   {
      typedef T type;
   };

   template <class T> 
   struct remove_pointer_imp<T*>
   {
      typedef T type;
   };

   template <class T, bool b> 
   struct remove_pointer_imp3
   {
      typedef typename remove_pointer_imp<typename Pothos::remove_cv<T>::type>::type type;
   };

   template <class T> 
   struct remove_pointer_imp3<T, false>
   {
      typedef T type;
   };

   template <class T> 
   struct remove_pointer_imp2
   {
      typedef typename remove_pointer_imp3<T, ::Pothos::is_pointer<T>::value>::type type;
   };
}

POTHOS_TT_AUX_TYPE_TRAIT_DEF1(remove_pointer,T,typename Pothos::detail::remove_pointer_imp2<T>::type)

#elif !defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

POTHOS_TT_AUX_TYPE_TRAIT_DEF1(remove_pointer,T,T)
POTHOS_TT_AUX_TYPE_TRAIT_PARTIAL_SPEC1_1(typename T,remove_pointer,T*,T)
POTHOS_TT_AUX_TYPE_TRAIT_PARTIAL_SPEC1_1(typename T,remove_pointer,T* const,T)
POTHOS_TT_AUX_TYPE_TRAIT_PARTIAL_SPEC1_1(typename T,remove_pointer,T* volatile,T)
POTHOS_TT_AUX_TYPE_TRAIT_PARTIAL_SPEC1_1(typename T,remove_pointer,T* const volatile,T)

#elif !POTHOS_WORKAROUND(POTHOS_MSVC,<=1300)

POTHOS_TT_AUX_TYPE_TRAIT_DEF1(remove_pointer,T,typename Pothos::detail::remove_pointer_impl<T>::type)

#endif

} // namespace boost

#include <Pothos/serialization/impl/type_traits/detail/type_trait_undef.hpp>

#endif // BOOST_TT_REMOVE_POINTER_HPP_INCLUDED
