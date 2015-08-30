
// NO INCLUDE GUARDS, THE HEADER IS INTENDED FOR MULTIPLE INCLUSION

// Copyright Aleksey Gurtovoy 2002-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)

// $Source$
// $Date: 2011-04-25 05:26:48 -0700 (Mon, 25 Apr 2011) $
// $Revision: 71481 $

#include <Pothos/serialization/impl/type_traits/detail/template_arity_spec.hpp>
#include <Pothos/serialization/impl/type_traits/integral_constant.hpp>
#include <Pothos/serialization/impl/mpl/aux_/lambda_support.hpp>
#include <Pothos/serialization/impl/mpl/size_t.hpp>

#include <cstddef>

#if !defined(POTHOS_MSVC) || POTHOS_MSVC >= 1300
#   define POTHOS_TT_AUX_SIZE_T_BASE(C) public ::Pothos::integral_constant<std::size_t,C>
#   define POTHOS_TT_AUX_SIZE_T_TRAIT_VALUE_DECL(C) /**/
#else
#   define POTHOS_TT_AUX_SIZE_T_BASE(C) public ::Pothos::mpl::size_t<C>
#   define POTHOS_TT_AUX_SIZE_T_TRAIT_VALUE_DECL(C) \
    typedef ::Pothos::mpl::size_t<C> base_; \
    using base_::value; \
    /**/
#endif


#define POTHOS_TT_AUX_SIZE_T_TRAIT_DEF1(trait,T,C) \
template< typename T > struct trait \
    : POTHOS_TT_AUX_SIZE_T_BASE(C) \
{ \
public:\
    POTHOS_TT_AUX_SIZE_T_TRAIT_VALUE_DECL(C) \
    POTHOS_MPL_AUX_LAMBDA_SUPPORT(1,trait,(T)) \
}; \
\
POTHOS_TT_AUX_TEMPLATE_ARITY_SPEC(1,trait) \
/**/

#define POTHOS_TT_AUX_SIZE_T_TRAIT_SPEC1(trait,spec,C) \
template<> struct trait<spec> \
    : POTHOS_TT_AUX_SIZE_T_BASE(C) \
{ \
public:\
    POTHOS_TT_AUX_SIZE_T_TRAIT_VALUE_DECL(C) \
    POTHOS_MPL_AUX_LAMBDA_SUPPORT_SPEC(1,trait,(spec)) \
}; \
/**/

#define POTHOS_TT_AUX_SIZE_T_TRAIT_PARTIAL_SPEC1_1(param,trait,spec,C) \
template< param > struct trait<spec> \
    : POTHOS_TT_AUX_SIZE_T_BASE(C) \
{ \
}; \
/**/
