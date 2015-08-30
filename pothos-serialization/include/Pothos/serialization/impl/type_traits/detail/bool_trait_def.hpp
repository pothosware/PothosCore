
// NO INCLUDE GUARDS, THE HEADER IS INTENDED FOR MULTIPLE INCLUSION

// Copyright Aleksey Gurtovoy 2002-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)

// $Source$
// $Date: 2011-10-09 15:28:33 -0700 (Sun, 09 Oct 2011) $
// $Revision: 74865 $

#include <Pothos/serialization/impl/type_traits/detail/template_arity_spec.hpp>
#include <Pothos/serialization/impl/type_traits/integral_constant.hpp>
#include <Pothos/serialization/impl/mpl/bool.hpp>
#include <Pothos/serialization/impl/mpl/aux_/lambda_support.hpp>
#include <Pothos/serialization/impl/config.hpp>

//
// Unfortunately some libraries have started using this header without
// cleaning up afterwards: so we'd better undef the macros just in case 
// they've been defined already....
//
#ifdef POTHOS_TT_AUX_BOOL_TRAIT_VALUE_DECL
#undef POTHOS_TT_AUX_BOOL_TRAIT_VALUE_DECL
#undef POTHOS_TT_AUX_BOOL_C_BASE
#undef POTHOS_TT_AUX_BOOL_TRAIT_DEF1
#undef POTHOS_TT_AUX_BOOL_TRAIT_DEF2
#undef POTHOS_TT_AUX_BOOL_TRAIT_SPEC1
#undef POTHOS_TT_AUX_BOOL_TRAIT_SPEC2
#undef POTHOS_TT_AUX_BOOL_TRAIT_IMPL_SPEC1
#undef POTHOS_TT_AUX_BOOL_TRAIT_IMPL_SPEC2
#undef POTHOS_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_1
#undef POTHOS_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_2
#undef POTHOS_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC2_1
#undef POTHOS_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC2_2
#undef POTHOS_TT_AUX_BOOL_TRAIT_IMPL_PARTIAL_SPEC2_1
#undef POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1
#endif

#if defined(__SUNPRO_CC) && (__SUNPRO_CC < 0x570)
#   define POTHOS_TT_AUX_BOOL_TRAIT_VALUE_DECL(C) \
    typedef ::Pothos::integral_constant<bool,C> type; \
    enum { value = type::value }; \
    /**/
#   define POTHOS_TT_AUX_BOOL_C_BASE(C)

#elif defined(POTHOS_MSVC) && POTHOS_MSVC < 1300

#   define POTHOS_TT_AUX_BOOL_TRAIT_VALUE_DECL(C) \
    typedef ::Pothos::integral_constant<bool,C> base_; \
    using base_::value; \
    /**/

#endif

#ifndef POTHOS_TT_AUX_BOOL_TRAIT_VALUE_DECL
#   define POTHOS_TT_AUX_BOOL_TRAIT_VALUE_DECL(C) /**/
#endif

#ifndef POTHOS_TT_AUX_BOOL_C_BASE
#   define POTHOS_TT_AUX_BOOL_C_BASE(C) : public ::Pothos::integral_constant<bool,C>
#endif 


#define POTHOS_TT_AUX_BOOL_TRAIT_DEF1(trait,T,C) \
template< typename T > struct trait \
    POTHOS_TT_AUX_BOOL_C_BASE(C) \
{ \
public:\
    POTHOS_TT_AUX_BOOL_TRAIT_VALUE_DECL(C) \
    POTHOS_MPL_AUX_LAMBDA_SUPPORT(1,trait,(T)) \
}; \
\
POTHOS_TT_AUX_TEMPLATE_ARITY_SPEC(1,trait) \
/**/


#define POTHOS_TT_AUX_BOOL_TRAIT_DEF2(trait,T1,T2,C) \
template< typename T1, typename T2 > struct trait \
    POTHOS_TT_AUX_BOOL_C_BASE(C) \
{ \
public:\
    POTHOS_TT_AUX_BOOL_TRAIT_VALUE_DECL(C) \
    POTHOS_MPL_AUX_LAMBDA_SUPPORT(2,trait,(T1,T2)) \
}; \
\
POTHOS_TT_AUX_TEMPLATE_ARITY_SPEC(2,trait) \
/**/

#define POTHOS_TT_AUX_BOOL_TRAIT_DEF3(trait,T1,T2,T3,C) \
template< typename T1, typename T2, typename T3 > struct trait \
    POTHOS_TT_AUX_BOOL_C_BASE(C) \
{ \
public:\
    POTHOS_TT_AUX_BOOL_TRAIT_VALUE_DECL(C) \
    POTHOS_MPL_AUX_LAMBDA_SUPPORT(3,trait,(T1,T2,T3)) \
}; \
\
POTHOS_TT_AUX_TEMPLATE_ARITY_SPEC(3,trait) \
/**/

#define POTHOS_TT_AUX_BOOL_TRAIT_SPEC1(trait,sp,C) \
template<> struct trait< sp > \
    POTHOS_TT_AUX_BOOL_C_BASE(C) \
{ \
public:\
    POTHOS_TT_AUX_BOOL_TRAIT_VALUE_DECL(C) \
    POTHOS_MPL_AUX_LAMBDA_SUPPORT_SPEC(1,trait,(sp)) \
}; \
/**/

#define POTHOS_TT_AUX_BOOL_TRAIT_SPEC2(trait,sp1,sp2,C) \
template<> struct trait< sp1,sp2 > \
    POTHOS_TT_AUX_BOOL_C_BASE(C) \
{ \
public:\
    POTHOS_TT_AUX_BOOL_TRAIT_VALUE_DECL(C) \
    POTHOS_MPL_AUX_LAMBDA_SUPPORT_SPEC(2,trait,(sp1,sp2)) \
}; \
/**/

#define POTHOS_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(trait,sp,C) \
template<> struct trait##_impl< sp > \
{ \
public:\
    POTHOS_STATIC_CONSTANT(bool, value = (C)); \
}; \
/**/

#define POTHOS_TT_AUX_BOOL_TRAIT_IMPL_SPEC2(trait,sp1,sp2,C) \
template<> struct trait##_impl< sp1,sp2 > \
{ \
public:\
    POTHOS_STATIC_CONSTANT(bool, value = (C)); \
}; \
/**/

#define POTHOS_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_1(param,trait,sp,C) \
template< param > struct trait< sp > \
    POTHOS_TT_AUX_BOOL_C_BASE(C) \
{ \
public:\
    POTHOS_TT_AUX_BOOL_TRAIT_VALUE_DECL(C) \
}; \
/**/

#define POTHOS_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_2(param1,param2,trait,sp,C) \
template< param1, param2 > struct trait< sp > \
    POTHOS_TT_AUX_BOOL_C_BASE(C) \
{ \
public:\
    POTHOS_TT_AUX_BOOL_TRAIT_VALUE_DECL(C) \
}; \
/**/

#define POTHOS_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC2_1(param,trait,sp1,sp2,C) \
template< param > struct trait< sp1,sp2 > \
    POTHOS_TT_AUX_BOOL_C_BASE(C) \
{ \
public:\
    POTHOS_TT_AUX_BOOL_TRAIT_VALUE_DECL(C) \
    POTHOS_MPL_AUX_LAMBDA_SUPPORT_SPEC(2,trait,(sp1,sp2)) \
}; \
/**/

#define POTHOS_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC2_2(param1,param2,trait,sp1,sp2,C) \
template< param1, param2 > struct trait< sp1,sp2 > \
    POTHOS_TT_AUX_BOOL_C_BASE(C) \
{ \
public:\
    POTHOS_TT_AUX_BOOL_TRAIT_VALUE_DECL(C) \
}; \
/**/

#define POTHOS_TT_AUX_BOOL_TRAIT_IMPL_PARTIAL_SPEC2_1(param,trait,sp1,sp2,C) \
template< param > struct trait##_impl< sp1,sp2 > \
{ \
public:\
    POTHOS_STATIC_CONSTANT(bool, value = (C)); \
}; \
/**/

#ifndef POTHOS_NO_CV_SPECIALIZATIONS
#   define POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1(trait,sp,value) \
    POTHOS_TT_AUX_BOOL_TRAIT_SPEC1(trait,sp,value) \
    POTHOS_TT_AUX_BOOL_TRAIT_SPEC1(trait,sp const,value) \
    POTHOS_TT_AUX_BOOL_TRAIT_SPEC1(trait,sp volatile,value) \
    POTHOS_TT_AUX_BOOL_TRAIT_SPEC1(trait,sp const volatile,value) \
    /**/
#else
#   define POTHOS_TT_AUX_BOOL_TRAIT_CV_SPEC1(trait,sp,value) \
    POTHOS_TT_AUX_BOOL_TRAIT_SPEC1(trait,sp,value) \
    /**/
#endif
