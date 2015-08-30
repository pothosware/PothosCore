
//  Copyright 2001-2003 Aleksey Gurtovoy.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef POTHOS_TT_BROKEN_COMPILER_SPEC_HPP_INCLUDED
#define POTHOS_TT_BROKEN_COMPILER_SPEC_HPP_INCLUDED

#include <Pothos/serialization/impl/mpl/aux_/lambda_support.hpp>
#include <Pothos/serialization/impl/config.hpp>

// these are needed regardless of BOOST_TT_NO_BROKEN_COMPILER_SPEC 
#if defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
namespace Pothos { namespace detail {
template< typename T > struct remove_const_impl     { typedef T type; };
template< typename T > struct remove_volatile_impl  { typedef T type; };
template< typename T > struct remove_pointer_impl   { typedef T type; };
template< typename T > struct remove_reference_impl { typedef T type; };
typedef int invoke_POTHOS_TT_BROKEN_COMPILER_SPEC_outside_all_namespaces;
}}
#endif

// agurt, 27/jun/03: disable the workaround if user defined 
// BOOST_TT_NO_BROKEN_COMPILER_SPEC
#if    !defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION) \
    || defined(POTHOS_TT_NO_BROKEN_COMPILER_SPEC)

#   define POTHOS_TT_BROKEN_COMPILER_SPEC(T) /**/

#else

// same as BOOST_TT_AUX_TYPE_TRAIT_IMPL_SPEC1 macro, except that it
// never gets #undef-ined
#   define POTHOS_TT_AUX_BROKEN_TYPE_TRAIT_SPEC1(trait,spec,result) \
template<> struct trait##_impl<spec> \
{ \
    typedef result type; \
}; \
/**/

#   define POTHOS_TT_AUX_REMOVE_CONST_VOLATILE_RANK1_SPEC(T)                         \
    POTHOS_TT_AUX_BROKEN_TYPE_TRAIT_SPEC1(remove_const,T const,T)                    \
    POTHOS_TT_AUX_BROKEN_TYPE_TRAIT_SPEC1(remove_const,T const volatile,T volatile)  \
    POTHOS_TT_AUX_BROKEN_TYPE_TRAIT_SPEC1(remove_volatile,T volatile,T)              \
    POTHOS_TT_AUX_BROKEN_TYPE_TRAIT_SPEC1(remove_volatile,T const volatile,T const)  \
    /**/

#   define POTHOS_TT_AUX_REMOVE_PTR_REF_RANK_1_SPEC(T)                               \
    POTHOS_TT_AUX_BROKEN_TYPE_TRAIT_SPEC1(remove_pointer,T*,T)                       \
    POTHOS_TT_AUX_BROKEN_TYPE_TRAIT_SPEC1(remove_pointer,T*const,T)                  \
    POTHOS_TT_AUX_BROKEN_TYPE_TRAIT_SPEC1(remove_pointer,T*volatile,T)               \
    POTHOS_TT_AUX_BROKEN_TYPE_TRAIT_SPEC1(remove_pointer,T*const volatile,T)         \
    POTHOS_TT_AUX_BROKEN_TYPE_TRAIT_SPEC1(remove_reference,T&,T)                     \
    /**/

#   define POTHOS_TT_AUX_REMOVE_PTR_REF_RANK_2_SPEC(T)                               \
    POTHOS_TT_AUX_REMOVE_PTR_REF_RANK_1_SPEC(T)                                      \
    POTHOS_TT_AUX_REMOVE_PTR_REF_RANK_1_SPEC(T const)                                \
    POTHOS_TT_AUX_REMOVE_PTR_REF_RANK_1_SPEC(T volatile)                             \
    POTHOS_TT_AUX_REMOVE_PTR_REF_RANK_1_SPEC(T const volatile)                       \
    /**/

#   define POTHOS_TT_AUX_REMOVE_ALL_RANK_1_SPEC(T)                                   \
    POTHOS_TT_AUX_REMOVE_PTR_REF_RANK_2_SPEC(T)                                      \
    POTHOS_TT_AUX_REMOVE_CONST_VOLATILE_RANK1_SPEC(T)                                \
    /**/

#   define POTHOS_TT_AUX_REMOVE_ALL_RANK_2_SPEC(T)                                   \
    POTHOS_TT_AUX_REMOVE_ALL_RANK_1_SPEC(T*)                                         \
    POTHOS_TT_AUX_REMOVE_ALL_RANK_1_SPEC(T const*)                                   \
    POTHOS_TT_AUX_REMOVE_ALL_RANK_1_SPEC(T volatile*)                                \
    POTHOS_TT_AUX_REMOVE_ALL_RANK_1_SPEC(T const volatile*)                          \
    /**/

#   define POTHOS_TT_BROKEN_COMPILER_SPEC(T)                                         \
    namespace Pothos { namespace detail {                                            \
    typedef invoke_POTHOS_TT_BROKEN_COMPILER_SPEC_outside_all_namespaces             \
      please_invoke_POTHOS_TT_BROKEN_COMPILER_SPEC_outside_all_namespaces;           \
    POTHOS_TT_AUX_REMOVE_ALL_RANK_1_SPEC(T)                                          \
    POTHOS_TT_AUX_REMOVE_ALL_RANK_2_SPEC(T)                                          \
    POTHOS_TT_AUX_REMOVE_ALL_RANK_2_SPEC(T*)                                         \
    POTHOS_TT_AUX_REMOVE_ALL_RANK_2_SPEC(T const*)                                   \
    POTHOS_TT_AUX_REMOVE_ALL_RANK_2_SPEC(T volatile*)                                \
    POTHOS_TT_AUX_REMOVE_ALL_RANK_2_SPEC(T const volatile*)                          \
    }}                                                                              \
    /**/

#   include <Pothos/serialization/impl/type_traits/detail/type_trait_undef.hpp>

#endif // BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

POTHOS_TT_BROKEN_COMPILER_SPEC(bool)
POTHOS_TT_BROKEN_COMPILER_SPEC(char)
#ifndef POTHOS_NO_INTRINSIC_WCHAR_T
POTHOS_TT_BROKEN_COMPILER_SPEC(wchar_t)
#endif
POTHOS_TT_BROKEN_COMPILER_SPEC(signed char)
POTHOS_TT_BROKEN_COMPILER_SPEC(unsigned char)
POTHOS_TT_BROKEN_COMPILER_SPEC(signed short)
POTHOS_TT_BROKEN_COMPILER_SPEC(unsigned short)
POTHOS_TT_BROKEN_COMPILER_SPEC(signed int)
POTHOS_TT_BROKEN_COMPILER_SPEC(unsigned int)
POTHOS_TT_BROKEN_COMPILER_SPEC(signed long)
POTHOS_TT_BROKEN_COMPILER_SPEC(unsigned long)
POTHOS_TT_BROKEN_COMPILER_SPEC(float)
POTHOS_TT_BROKEN_COMPILER_SPEC(double)
//BOOST_TT_BROKEN_COMPILER_SPEC(long double)

// for backward compatibility
#define POTHOS_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(T) \
    POTHOS_TT_BROKEN_COMPILER_SPEC(T) \
/**/

#endif // BOOST_TT_BROKEN_COMPILER_SPEC_HPP_INCLUDED
