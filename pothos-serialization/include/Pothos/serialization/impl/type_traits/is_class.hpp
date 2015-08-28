//  (C) Copyright Dave Abrahams, Steve Cleary, Beman Dawes, Howard
//  Hinnant & John Maddock 2000-2003.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.


#ifndef POTHOS_TT_IS_CLASS_HPP_INCLUDED
#define POTHOS_TT_IS_CLASS_HPP_INCLUDED

#include <Pothos/serialization/impl/type_traits/config.hpp>
#include <Pothos/serialization/impl/type_traits/intrinsics.hpp>
#ifndef POTHOS_IS_CLASS
#   include <Pothos/serialization/impl/type_traits/is_union.hpp>
#   include <Pothos/serialization/impl/type_traits/detail/ice_and.hpp>
#   include <Pothos/serialization/impl/type_traits/detail/ice_not.hpp>

#ifdef POTHOS_TT_HAS_CONFORMING_IS_CLASS_IMPLEMENTATION
#   include <Pothos/serialization/impl/type_traits/detail/yes_no_type.hpp>
#else
#   include <Pothos/serialization/impl/type_traits/is_scalar.hpp>
#   include <Pothos/serialization/impl/type_traits/is_array.hpp>
#   include <Pothos/serialization/impl/type_traits/is_reference.hpp>
#   include <Pothos/serialization/impl/type_traits/is_void.hpp>
#   include <Pothos/serialization/impl/type_traits/is_function.hpp>
#endif

#endif // BOOST_IS_CLASS

#ifdef __EDG_VERSION__
#   include <Pothos/serialization/impl/type_traits/remove_cv.hpp>
#endif

// should be the last #include
#include <Pothos/serialization/impl/type_traits/detail/bool_trait_def.hpp>

namespace Pothos {

namespace detail {

#ifndef POTHOS_IS_CLASS
#ifdef POTHOS_TT_HAS_CONFORMING_IS_CLASS_IMPLEMENTATION

// This is actually the conforming implementation which works with
// abstract classes.  However, enough compilers have trouble with
// it that most will use the one in
// boost/type_traits/object_traits.hpp. This implementation
// actually works with VC7.0, but other interactions seem to fail
// when we use it.

// is_class<> metafunction due to Paul Mensonides
// (leavings@attbi.com). For more details:
// http://groups.google.com/groups?hl=en&selm=000001c1cc83%24e154d5e0%247772e50c%40c161550a&rnum=1
#if defined(__GNUC__)  && !defined(__EDG_VERSION__)

template <class U> ::Pothos::type_traits::yes_type is_class_tester(void(U::*)(void));
template <class U> ::Pothos::type_traits::no_type is_class_tester(...);

template <typename T>
struct is_class_impl
{

    POTHOS_STATIC_CONSTANT(bool, value =
        (::Pothos::type_traits::ice_and<
            sizeof(is_class_tester<T>(0)) == sizeof(::Pothos::type_traits::yes_type),
            ::Pothos::type_traits::ice_not< ::Pothos::is_union<T>::value >::value
        >::value)
        );
};

#else

template <typename T>
struct is_class_impl
{
    template <class U> static ::Pothos::type_traits::yes_type is_class_tester(void(U::*)(void));
    template <class U> static ::Pothos::type_traits::no_type is_class_tester(...);

    POTHOS_STATIC_CONSTANT(bool, value =
        (::Pothos::type_traits::ice_and<
            sizeof(is_class_tester<T>(0)) == sizeof(::Pothos::type_traits::yes_type),
            ::Pothos::type_traits::ice_not< ::Pothos::is_union<T>::value >::value
        >::value)
        );
};

#endif

#else

template <typename T>
struct is_class_impl
{
#   ifndef POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION
    POTHOS_STATIC_CONSTANT(bool, value =
    (::Pothos::type_traits::ice_and<
        ::Pothos::type_traits::ice_not< ::Pothos::is_union<T>::value >::value,
        ::Pothos::type_traits::ice_not< ::Pothos::is_scalar<T>::value >::value,
        ::Pothos::type_traits::ice_not< ::Pothos::is_array<T>::value >::value,
        ::Pothos::type_traits::ice_not< ::Pothos::is_reference<T>::value>::value,
        ::Pothos::type_traits::ice_not< ::Pothos::is_void<T>::value >::value,
        ::Pothos::type_traits::ice_not< ::Pothos::is_function<T>::value >::value
        >::value));
#   else
    POTHOS_STATIC_CONSTANT(bool, value =
    (::Pothos::type_traits::ice_and<
        ::Pothos::type_traits::ice_not< ::Pothos::is_union<T>::value >::value,
        ::Pothos::type_traits::ice_not< ::Pothos::is_scalar<T>::value >::value,
        ::Pothos::type_traits::ice_not< ::Pothos::is_array<T>::value >::value,
        ::Pothos::type_traits::ice_not< ::Pothos::is_reference<T>::value>::value,
        ::Pothos::type_traits::ice_not< ::Pothos::is_void<T>::value >::value
        >::value));
#   endif
};

# endif // BOOST_TT_HAS_CONFORMING_IS_CLASS_IMPLEMENTATION
# else // BOOST_IS_CLASS
template <typename T>
struct is_class_impl
{
    POTHOS_STATIC_CONSTANT(bool, value = POTHOS_IS_CLASS(T));
};
# endif // BOOST_IS_CLASS

} // namespace detail

# ifdef __EDG_VERSION__
POTHOS_TT_AUX_BOOL_TRAIT_DEF1(
   is_class,T, Pothos::detail::is_class_impl<typename Pothos::remove_cv<T>::type>::value)
# else 
POTHOS_TT_AUX_BOOL_TRAIT_DEF1(is_class,T,::Pothos::detail::is_class_impl<T>::value)
# endif
    
} // namespace boost

#include <Pothos/serialization/impl/type_traits/detail/bool_trait_undef.hpp>

#endif // BOOST_TT_IS_CLASS_HPP_INCLUDED
