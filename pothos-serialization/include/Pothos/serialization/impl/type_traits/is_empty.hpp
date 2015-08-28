
// (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef POTHOS_TT_IS_EMPTY_HPP_INCLUDED
#define POTHOS_TT_IS_EMPTY_HPP_INCLUDED

#include <Pothos/serialization/impl/type_traits/is_convertible.hpp>
#include <Pothos/serialization/impl/type_traits/detail/ice_or.hpp>
#include <Pothos/serialization/impl/type_traits/config.hpp>
#include <Pothos/serialization/impl/type_traits/intrinsics.hpp>

#ifndef POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION
#   include <Pothos/serialization/impl/type_traits/remove_cv.hpp>
#   include <Pothos/serialization/impl/type_traits/is_class.hpp>
#   include <Pothos/serialization/impl/type_traits/add_reference.hpp>
#else
#   include <Pothos/serialization/impl/type_traits/is_reference.hpp>
#   include <Pothos/serialization/impl/type_traits/is_pointer.hpp>
#   include <Pothos/serialization/impl/type_traits/is_member_pointer.hpp>
#   include <Pothos/serialization/impl/type_traits/is_array.hpp>
#   include <Pothos/serialization/impl/type_traits/is_void.hpp>
#   include <Pothos/serialization/impl/type_traits/detail/ice_and.hpp>
#   include <Pothos/serialization/impl/type_traits/detail/ice_not.hpp>
#endif

// should be always the last #include directive
#include <Pothos/serialization/impl/type_traits/detail/bool_trait_def.hpp>

#ifndef POTHOS_INTERNAL_IS_EMPTY
#define POTHOS_INTERNAL_IS_EMPTY(T) false
#else
#define POTHOS_INTERNAL_IS_EMPTY(T) POTHOS_IS_EMPTY(T)
#endif

namespace Pothos {

namespace detail {

#ifndef POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION

#ifdef POTHOS_MSVC
#pragma warning(push)
#pragma warning(disable:4624) // destructor could not be generated
#endif

template <typename T>
struct empty_helper_t1 : public T
{
    empty_helper_t1();  // hh compiler bug workaround
    int i[256];
private:
   // suppress compiler warnings:
   empty_helper_t1(const empty_helper_t1&);
   empty_helper_t1& operator=(const empty_helper_t1&);
};

#ifdef POTHOS_MSVC
#pragma warning(pop)
#endif

struct empty_helper_t2 { int i[256]; };

#if !POTHOS_WORKAROUND(__BORLANDC__, < 0x600)

template <typename T, bool is_a_class = false>
struct empty_helper
{
    POTHOS_STATIC_CONSTANT(bool, value = false);
};

template <typename T>
struct empty_helper<T, true>
{
    POTHOS_STATIC_CONSTANT(
        bool, value = (sizeof(empty_helper_t1<T>) == sizeof(empty_helper_t2))
        );
};

template <typename T>
struct is_empty_impl
{
    typedef typename remove_cv<T>::type cvt;
    POTHOS_STATIC_CONSTANT(
        bool, value = (
            ::Pothos::type_traits::ice_or<
              ::Pothos::detail::empty_helper<cvt,::Pothos::is_class<T>::value>::value
              , POTHOS_INTERNAL_IS_EMPTY(cvt)
            >::value
            ));
};

#else // __BORLANDC__

template <typename T, bool is_a_class, bool convertible_to_int>
struct empty_helper
{
    POTHOS_STATIC_CONSTANT(bool, value = false);
};

template <typename T>
struct empty_helper<T, true, false>
{
    POTHOS_STATIC_CONSTANT(bool, value = (
        sizeof(empty_helper_t1<T>) == sizeof(empty_helper_t2)
        ));
};

template <typename T>
struct is_empty_impl
{
   typedef typename remove_cv<T>::type cvt;
   typedef typename add_reference<T>::type r_type;

   POTHOS_STATIC_CONSTANT(
       bool, value = (
           ::Pothos::type_traits::ice_or<
              ::Pothos::detail::empty_helper<
                  cvt
                , ::Pothos::is_class<T>::value
                , ::Pothos::is_convertible< r_type,int>::value
              >::value
              , POTHOS_INTERNAL_IS_EMPTY(cvt)
           >::value));
};

#endif // __BORLANDC__

#else // BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

#ifdef POTHOS_MSVC6_MEMBER_TEMPLATES

template <typename T>
struct empty_helper_t1 : public T
{
   empty_helper_t1();
   int i[256];
};

struct empty_helper_t2 { int i[256]; };

template <typename T>
struct empty_helper_base
{
   enum { value = (sizeof(empty_helper_t1<T>) == sizeof(empty_helper_t2)) };
};

template <typename T>
struct empty_helper_nonbase
{
   enum { value = false };
};

template <bool base>
struct empty_helper_chooser
{
   template <typename T> struct result_
   {
      typedef empty_helper_nonbase<T> type;
   };
};

template <>
struct empty_helper_chooser<true>
{
   template <typename T> struct result_
   {
      typedef empty_helper_base<T> type;
   };
};

template <typename T>
struct is_empty_impl
{
   typedef ::Pothos::detail::empty_helper_chooser<
      ::Pothos::type_traits::ice_and<
         ::Pothos::type_traits::ice_not< ::Pothos::is_reference<T>::value >::value,
         ::Pothos::type_traits::ice_not< ::Pothos::is_convertible<T,double>::value >::value,
         ::Pothos::type_traits::ice_not< ::Pothos::is_pointer<T>::value >::value,
         ::Pothos::type_traits::ice_not< ::Pothos::is_member_pointer<T>::value >::value,
         ::Pothos::type_traits::ice_not< ::Pothos::is_array<T>::value >::value,
         ::Pothos::type_traits::ice_not< ::Pothos::is_void<T>::value >::value,
         ::Pothos::type_traits::ice_not<
            ::Pothos::is_convertible<T,void const volatile*>::value
            >::value
      >::value > chooser;

   typedef typename chooser::template result_<T> result;
   typedef typename result::type eh_type;

   POTHOS_STATIC_CONSTANT(bool, value =
      (::Pothos::type_traits::ice_or<eh_type::value, POTHOS_INTERNAL_IS_EMPTY(T)>::value));
};

#else

template <typename T> struct is_empty_impl
{
    POTHOS_STATIC_CONSTANT(bool, value = POTHOS_INTERNAL_IS_EMPTY(T));
};

#endif  // BOOST_MSVC6_MEMBER_TEMPLATES

#endif  // BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

// these help when the compiler has no partial specialization support:
POTHOS_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_empty,void,false)
#ifndef POTHOS_NO_CV_VOID_SPECIALIZATIONS
POTHOS_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_empty,void const,false)
POTHOS_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_empty,void volatile,false)
POTHOS_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_empty,void const volatile,false)
#endif

} // namespace detail

POTHOS_TT_AUX_BOOL_TRAIT_DEF1(is_empty,T,::Pothos::detail::is_empty_impl<T>::value)

} // namespace boost

#include <Pothos/serialization/impl/type_traits/detail/bool_trait_undef.hpp>

#undef POTHOS_INTERNAL_IS_EMPTY

#endif // BOOST_TT_IS_EMPTY_HPP_INCLUDED

