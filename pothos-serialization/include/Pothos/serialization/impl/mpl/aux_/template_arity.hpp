
#if !defined(POTHOS_PP_IS_ITERATING)

///// header body

#ifndef POTHOS_MPL_AUX_TEMPLATE_ARITY_HPP_INCLUDED
#define POTHOS_MPL_AUX_TEMPLATE_ARITY_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2001-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: template_arity.hpp 61584 2010-04-26 18:48:26Z agurtovoy $
// $Date: 2010-04-26 11:48:26 -0700 (Mon, 26 Apr 2010) $
// $Revision: 61584 $

#include <Pothos/serialization/impl/mpl/aux_/config/ttp.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/lambda.hpp>

#if !defined(POTHOS_MPL_PREPROCESSING_MODE)
#   include <Pothos/serialization/impl/mpl/aux_/template_arity_fwd.hpp>
#   include <Pothos/serialization/impl/mpl/int.hpp>
#   if !defined(POTHOS_MPL_CFG_NO_FULL_LAMBDA_SUPPORT)
#   if defined(POTHOS_MPL_CFG_EXTENDED_TEMPLATE_PARAMETERS_MATCHING)
#       include <Pothos/serialization/impl/mpl/aux_/type_wrapper.hpp>
#   endif
#   else
#       include <Pothos/serialization/impl/mpl/aux_/has_rebind.hpp>
#   endif
#endif

#include <Pothos/serialization/impl/mpl/aux_/config/static_constant.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/use_preprocessed.hpp>

#if !defined(POTHOS_MPL_CFG_NO_PREPROCESSED_HEADERS) \
    && !defined(POTHOS_MPL_PREPROCESSING_MODE)

#   define POTHOS_MPL_PREPROCESSED_HEADER template_arity.hpp
#   include <Pothos/serialization/impl/mpl/aux_/include_preprocessed.hpp>

#else

#   if !defined(POTHOS_MPL_CFG_NO_FULL_LAMBDA_SUPPORT)
#   if defined(POTHOS_MPL_CFG_EXTENDED_TEMPLATE_PARAMETERS_MATCHING)

#   include <Pothos/serialization/impl/mpl/limits/arity.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/range.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/repeat.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/params.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/nttp_decl.hpp>

#   include <Pothos/serialization/impl/preprocessor/seq/fold_left.hpp>
#   include <Pothos/serialization/impl/preprocessor/comma_if.hpp>
#   include <Pothos/serialization/impl/preprocessor/iterate.hpp>
#   include <Pothos/serialization/impl/preprocessor/inc.hpp>
#   include <Pothos/serialization/impl/preprocessor/cat.hpp>

#   define AUX778076_ARITY POTHOS_PP_INC(POTHOS_MPL_LIMIT_METAFUNCTION_ARITY)

namespace Pothos { namespace mpl { namespace aux {

template< POTHOS_MPL_AUX_NTTP_DECL(int, N) > struct arity_tag
{
    typedef char (&type)[N + 1];
};

#   define AUX778076_MAX_ARITY_OP(unused, state, i_) \
    ( POTHOS_PP_CAT(C,i_) > 0 ? POTHOS_PP_CAT(C,i_) : state ) \
/**/

template<
      POTHOS_MPL_PP_PARAMS(AUX778076_ARITY, POTHOS_MPL_AUX_NTTP_DECL(int, C))
    >
struct max_arity
{
    POTHOS_STATIC_CONSTANT(int, value = 
          POTHOS_PP_SEQ_FOLD_LEFT(
              AUX778076_MAX_ARITY_OP
            , -1
            , POTHOS_MPL_PP_RANGE(1, AUX778076_ARITY)
            )
        );
};

#   undef AUX778076_MAX_ARITY_OP

arity_tag<0>::type arity_helper(...);

#   define POTHOS_PP_ITERATION_LIMITS (1, AUX778076_ARITY)
#   define POTHOS_PP_FILENAME_1 <Pothos/serialization/impl/mpl/aux_/template_arity.hpp>
#   include POTHOS_PP_ITERATE()

template< typename F, POTHOS_MPL_AUX_NTTP_DECL(int, N) >
struct template_arity_impl
{
    POTHOS_STATIC_CONSTANT(int, value = 
          sizeof(::Pothos::mpl::aux::arity_helper(type_wrapper<F>(),arity_tag<N>())) - 1
        );
};

#   define AUX778076_TEMPLATE_ARITY_IMPL_INVOCATION(unused, i_, F) \
    POTHOS_PP_COMMA_IF(i_) template_arity_impl<F,POTHOS_PP_INC(i_)>::value \
/**/

template< typename F >
struct template_arity
{
    POTHOS_STATIC_CONSTANT(int, value = (
          max_arity< POTHOS_MPL_PP_REPEAT(
              AUX778076_ARITY
            , AUX778076_TEMPLATE_ARITY_IMPL_INVOCATION
            , F
            ) >::value
        ));
        
    typedef mpl::int_<value> type;
};

#   undef AUX778076_TEMPLATE_ARITY_IMPL_INVOCATION

#   undef AUX778076_ARITY

}}}

#   endif // BOOST_MPL_CFG_EXTENDED_TEMPLATE_PARAMETERS_MATCHING
#   else // BOOST_MPL_CFG_NO_FULL_LAMBDA_SUPPORT

#   include <Pothos/serialization/impl/mpl/aux_/config/eti.hpp>

namespace Pothos { namespace mpl { namespace aux {

template< bool >
struct template_arity_impl
{
    template< typename F > struct result_
        : mpl::int_<-1>
    {
    };
};

template<>
struct template_arity_impl<true>
{
    template< typename F > struct result_
        : F::arity
    {
    };
};

template< typename F >
struct template_arity
    : template_arity_impl< ::Pothos::mpl::aux::has_rebind<F>::value >
        ::template result_<F>
{
};

#if defined(POTHOS_MPL_CFG_MSVC_ETI_BUG)
template<>
struct template_arity<int>
    : mpl::int_<-1>
{
};
#endif

}}}

#   endif // BOOST_MPL_CFG_NO_FULL_LAMBDA_SUPPORT

#endif // BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#endif // BOOST_MPL_AUX_TEMPLATE_ARITY_HPP_INCLUDED

///// iteration

#else
#define i_ POTHOS_PP_FRAME_ITERATION(1)

template<
      template< POTHOS_MPL_PP_PARAMS(i_, typename P) > class F
    , POTHOS_MPL_PP_PARAMS(i_, typename T)
    >
typename arity_tag<i_>::type
arity_helper(type_wrapper< F<POTHOS_MPL_PP_PARAMS(i_, T)> >, arity_tag<i_>);

#undef i_
#endif // BOOST_PP_IS_ITERATING
