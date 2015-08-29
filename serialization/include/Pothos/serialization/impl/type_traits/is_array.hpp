
//  (C) Copyright Dave Abrahams, Steve Cleary, Beman Dawes, Howard
//  Hinnant & John Maddock 2000.  
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.


// Some fixes for is_array are based on a newsgroup posting by Jonathan Lundquist.


#ifndef POTHOS_TT_IS_ARRAY_HPP_INCLUDED
#define POTHOS_TT_IS_ARRAY_HPP_INCLUDED

#include <Pothos/serialization/impl/type_traits/config.hpp>

#ifdef POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION
#   include <Pothos/serialization/impl/type_traits/detail/yes_no_type.hpp>
#   include <Pothos/serialization/impl/type_traits/detail/wrap.hpp>
#endif

#include <cstddef>

// should be the last #include
#include <Pothos/serialization/impl/type_traits/detail/bool_trait_def.hpp>

namespace Pothos {

#if defined( __CODEGEARC__ )
POTHOS_TT_AUX_BOOL_TRAIT_DEF1(is_array,T,__is_array(T))
#elif !defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
POTHOS_TT_AUX_BOOL_TRAIT_DEF1(is_array,T,false)
#if !defined(POTHOS_NO_ARRAY_TYPE_SPECIALIZATIONS)
POTHOS_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_2(typename T,std::size_t N,is_array,T[N],true)
POTHOS_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_2(typename T,std::size_t N,is_array,T const[N],true)
POTHOS_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_2(typename T,std::size_t N,is_array,T volatile[N],true)
POTHOS_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_2(typename T,std::size_t N,is_array,T const volatile[N],true)
#if !POTHOS_WORKAROUND(__BORLANDC__, < 0x600) && !defined(__IBMCPP__) &&  !POTHOS_WORKAROUND(__DMC__, POTHOS_TESTED_AT(0x840))
POTHOS_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_1(typename T,is_array,T[],true)
POTHOS_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_1(typename T,is_array,T const[],true)
POTHOS_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_1(typename T,is_array,T volatile[],true)
POTHOS_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_1(typename T,is_array,T const volatile[],true)
#endif
#endif

#else // BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

namespace detail {

using ::Pothos::type_traits::yes_type;
using ::Pothos::type_traits::no_type;
using ::Pothos::type_traits::wrap;

template< typename T > T(* is_array_tester1(wrap<T>) )(wrap<T>);
char POTHOS_TT_DECL is_array_tester1(...);

template< typename T> no_type is_array_tester2(T(*)(wrap<T>));
yes_type POTHOS_TT_DECL is_array_tester2(...);

template< typename T >
struct is_array_impl
{ 
    POTHOS_STATIC_CONSTANT(bool, value = 
        sizeof(::Pothos::detail::is_array_tester2(
            ::Pothos::detail::is_array_tester1(
                ::Pothos::type_traits::wrap<T>()
                )
        )) == 1
    );
};

#ifndef POTHOS_NO_CV_VOID_SPECIALIZATIONS
POTHOS_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_array,void,false)
POTHOS_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_array,void const,false)
POTHOS_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_array,void volatile,false)
POTHOS_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_array,void const volatile,false)
#endif

} // namespace detail

POTHOS_TT_AUX_BOOL_TRAIT_DEF1(is_array,T,::Pothos::detail::is_array_impl<T>::value)

#endif // BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

} // namespace boost

#include <Pothos/serialization/impl/type_traits/detail/bool_trait_undef.hpp>

#endif // BOOST_TT_IS_ARRAY_HPP_INCLUDED
