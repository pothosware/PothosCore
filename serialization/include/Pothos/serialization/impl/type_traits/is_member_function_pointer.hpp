
//  (C) Copyright Dave Abrahams, Steve Cleary, Beman Dawes, Howard
//  Hinnant & John Maddock 2000.  
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.


#ifndef POTHOS_TT_IS_MEMBER_FUNCTION_POINTER_HPP_INCLUDED
#define POTHOS_TT_IS_MEMBER_FUNCTION_POINTER_HPP_INCLUDED

#include <Pothos/serialization/impl/type_traits/config.hpp>
#include <Pothos/serialization/impl/detail/workaround.hpp>

#if !defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION) \
   && !POTHOS_WORKAROUND(__BORLANDC__, < 0x600) && !defined(POTHOS_TT_TEST_MS_FUNC_SIGS)
   //
   // Note: we use the "workaround" version for MSVC because it works for 
   // __stdcall etc function types, where as the partial specialisation
   // version does not do so.
   //
#   include <Pothos/serialization/impl/type_traits/detail/is_mem_fun_pointer_impl.hpp>
#   include <Pothos/serialization/impl/type_traits/remove_cv.hpp>
#else
#   include <Pothos/serialization/impl/type_traits/is_reference.hpp>
#   include <Pothos/serialization/impl/type_traits/is_array.hpp>
#   include <Pothos/serialization/impl/type_traits/detail/yes_no_type.hpp>
#   include <Pothos/serialization/impl/type_traits/detail/false_result.hpp>
#   include <Pothos/serialization/impl/type_traits/detail/ice_or.hpp>
#   include <Pothos/serialization/impl/type_traits/detail/is_mem_fun_pointer_tester.hpp>
#endif

// should be the last #include
#include <Pothos/serialization/impl/type_traits/detail/bool_trait_def.hpp>

namespace Pothos {

#if defined( __CODEGEARC__ )
POTHOS_TT_AUX_BOOL_TRAIT_DEF1(is_member_function_pointer,T,__is_member_function_pointer( T ))
#elif !defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION) && !POTHOS_WORKAROUND(__BORLANDC__, < 0x600) && !defined(POTHOS_TT_TEST_MS_FUNC_SIGS)

POTHOS_TT_AUX_BOOL_TRAIT_DEF1(
      is_member_function_pointer
    , T
    , ::Pothos::type_traits::is_mem_fun_pointer_impl<typename remove_cv<T>::type>::value
    )

#else

namespace detail {

#ifndef __BORLANDC__

template <bool>
struct is_mem_fun_pointer_select
    : public ::Pothos::type_traits::false_result
{
};

template <>
struct is_mem_fun_pointer_select<false>
{
    template <typename T> struct result_
    {
#if POTHOS_WORKAROUND(POTHOS_MSVC_FULL_VER, >= 140050000)
#pragma warning(push)
#pragma warning(disable:6334)
#endif
        static T* make_t;
        typedef result_<T> self_type;

        POTHOS_STATIC_CONSTANT(
            bool, value = (
                1 == sizeof(::Pothos::type_traits::is_mem_fun_pointer_tester(self_type::make_t))
            ));
#if POTHOS_WORKAROUND(POTHOS_MSVC_FULL_VER, >= 140050000)
#pragma warning(pop)
#endif
    };
};

template <typename T>
struct is_member_function_pointer_impl
    : public is_mem_fun_pointer_select<
          ::Pothos::type_traits::ice_or<
              ::Pothos::is_reference<T>::value
            , ::Pothos::is_array<T>::value
            >::value
        >::template result_<T>
{
};

#ifndef POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION
template <typename T>
struct is_member_function_pointer_impl<T&> : public false_type{};
#endif

#else // Borland C++

template <typename T>
struct is_member_function_pointer_impl
{
   static T* m_t;
   POTHOS_STATIC_CONSTANT(
              bool, value =
               (1 == sizeof(type_traits::is_mem_fun_pointer_tester(m_t))) );
};

template <typename T>
struct is_member_function_pointer_impl<T&>
{
   POTHOS_STATIC_CONSTANT(bool, value = false);
};

#endif

POTHOS_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_member_function_pointer,void,false)
#ifndef POTHOS_NO_CV_VOID_SPECIALIZATIONS
POTHOS_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_member_function_pointer,void const,false)
POTHOS_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_member_function_pointer,void volatile,false)
POTHOS_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_member_function_pointer,void const volatile,false)
#endif

} // namespace detail

POTHOS_TT_AUX_BOOL_TRAIT_DEF1(is_member_function_pointer,T,::Pothos::detail::is_member_function_pointer_impl<T>::value)

#endif // BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

} // namespace boost

#include <Pothos/serialization/impl/type_traits/detail/bool_trait_undef.hpp>

#endif // BOOST_TT_IS_MEMBER_FUNCTION_POINTER_HPP_INCLUDED
