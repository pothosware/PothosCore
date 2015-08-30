
#if !defined(POTHOS_PP_IS_ITERATING)

///// header body

#ifndef POTHOS_MPL_AUX_FULL_LAMBDA_HPP_INCLUDED
#define POTHOS_MPL_AUX_FULL_LAMBDA_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2001-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: full_lambda.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#if !defined(POTHOS_MPL_PREPROCESSING_MODE)
#   include <Pothos/serialization/impl/mpl/lambda_fwd.hpp>
#   include <Pothos/serialization/impl/mpl/bind_fwd.hpp>
#   include <Pothos/serialization/impl/mpl/protect.hpp>
#   include <Pothos/serialization/impl/mpl/quote.hpp>
#   include <Pothos/serialization/impl/mpl/arg.hpp>
#   include <Pothos/serialization/impl/mpl/bool.hpp>
#   include <Pothos/serialization/impl/mpl/int_fwd.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/template_arity.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/na_spec.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/ttp.hpp>
#   if defined(POTHOS_MPL_CFG_EXTENDED_TEMPLATE_PARAMETERS_MATCHING)
#       include <Pothos/serialization/impl/mpl/if.hpp>
#   endif
#endif

#include <Pothos/serialization/impl/mpl/aux_/lambda_arity_param.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/use_preprocessed.hpp>

#if !defined(POTHOS_MPL_CFG_NO_PREPROCESSED_HEADERS) \
    && !defined(POTHOS_MPL_PREPROCESSING_MODE)

#   define POTHOS_MPL_PREPROCESSED_HEADER full_lambda.hpp
#   include <Pothos/serialization/impl/mpl/aux_/include_preprocessed.hpp>

#else

#   include <Pothos/serialization/impl/mpl/limits/arity.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/default_params.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/params.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/enum.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/repeat.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/dmc_ambiguous_ctps.hpp>

#   include <Pothos/serialization/impl/preprocessor/iterate.hpp>
#   include <Pothos/serialization/impl/preprocessor/comma_if.hpp>
#   include <Pothos/serialization/impl/preprocessor/inc.hpp>
#   include <Pothos/serialization/impl/preprocessor/cat.hpp>

namespace Pothos { namespace mpl {

// local macros, #undef-ined at the end of the header
#   define AUX778076_LAMBDA_PARAMS(i_, param) \
    POTHOS_MPL_PP_PARAMS(i_, param) \
    /**/

#   define AUX778076_BIND_PARAMS(param) \
    POTHOS_MPL_PP_PARAMS( \
          POTHOS_MPL_LIMIT_METAFUNCTION_ARITY \
        , param \
        ) \
    /**/

#   define AUX778076_BIND_N_PARAMS(i_, param) \
    POTHOS_PP_COMMA_IF(i_) \
    POTHOS_MPL_PP_PARAMS(i_, param) \
    /**/

#   define AUX778076_ARITY_PARAM(param) \
    POTHOS_MPL_AUX_LAMBDA_ARITY_PARAM(param) \
    /**/


#define n_ POTHOS_MPL_LIMIT_METAFUNCTION_ARITY
namespace aux {

template<
      POTHOS_MPL_PP_DEFAULT_PARAMS(n_,bool C,false)
    >
struct lambda_or
    : true_
{
};

template<>
struct lambda_or< POTHOS_MPL_PP_ENUM(n_,false) >
    : false_
{
};

} // namespace aux
#undef n_

template<
      typename T
    , typename Tag
    AUX778076_ARITY_PARAM(typename Arity)
    >
struct lambda
{
    typedef false_ is_le;
    typedef T result_;
    typedef T type;
};

template<
      typename T
    >
struct is_lambda_expression
    : lambda<T>::is_le
{
};


template< int N, typename Tag >
struct lambda< arg<N>,Tag AUX778076_ARITY_PARAM(int_<-1>) >
{
    typedef true_ is_le;
    typedef mpl::arg<N> result_; // qualified for the sake of MIPSpro 7.41
    typedef mpl::protect<result_> type; 
};


#define POTHOS_PP_ITERATION_PARAMS_1 \
    (3,(0, POTHOS_MPL_LIMIT_METAFUNCTION_ARITY, <boost/mpl/aux_/full_lambda.hpp>))
#include POTHOS_PP_ITERATE()

/// special case for 'protect'
template< typename T, typename Tag >
struct lambda< mpl::protect<T>,Tag AUX778076_ARITY_PARAM(int_<1>) >
{
    typedef false_ is_le;
    typedef mpl::protect<T> result_;
    typedef result_ type;
};

/// specializations for the main 'bind' form
template<
      typename F, AUX778076_BIND_PARAMS(typename T)
    , typename Tag
    >
struct lambda<
          bind<F,AUX778076_BIND_PARAMS(T)>
        , Tag
        AUX778076_ARITY_PARAM(int_<POTHOS_PP_INC(POTHOS_MPL_LIMIT_METAFUNCTION_ARITY)>)
        >
{
    typedef false_ is_le;
    typedef bind<F, AUX778076_BIND_PARAMS(T)> result_;
    typedef result_ type;
};


#if defined(POTHOS_MPL_CFG_EXTENDED_TEMPLATE_PARAMETERS_MATCHING)

template<
      typename F
    , typename Tag1
    , typename Tag2
    , typename Arity
    >
struct lambda<
          lambda<F,Tag1,Arity>
        , Tag2
        , int_<3>
        >
{
    typedef lambda< F,Tag2 > l1;
    typedef lambda< Tag1,Tag2 > l2;

    typedef typename l1::is_le is_le;
    typedef bind1< quote1<aux::template_arity>, typename l1::result_ > arity_;
    typedef lambda< typename if_<is_le,arity_,Arity>::type,Tag2 > l3;
    
    typedef aux::le_result3<is_le, Tag2, mpl::lambda, l1, l2, l3> le_result_;
    typedef typename le_result_::result_ result_;
    typedef typename le_result_::type type;
};

#elif !defined(POTHOS_MPL_CFG_DMC_AMBIGUOUS_CTPS)

/// workaround for MWCW 8.3+/EDG < 303, leads to ambiguity on Digital Mars
template<
      typename F, typename Tag1, typename Tag2
    >
struct lambda<
          lambda< F,Tag1 >
        , Tag2
        >
{
    typedef lambda< F,Tag2 > l1;
    typedef lambda< Tag1,Tag2 > l2;
    
    typedef typename l1::is_le is_le;
    typedef aux::le_result2<is_le, Tag2, mpl::lambda, l1, l2> le_result_;
    typedef typename le_result_::result_ result_;
    typedef typename le_result_::type type;
};

#endif

#   undef AUX778076_ARITY_PARAM
#   undef AUX778076_BIND_N_PARAMS
#   undef AUX778076_BIND_PARAMS
#   undef AUX778076_LAMBDA_PARAMS

#if !defined(POTHOS_MPL_CFG_EXTENDED_TEMPLATE_PARAMETERS_MATCHING)
POTHOS_MPL_AUX_NA_SPEC(2, lambda)
#else
POTHOS_MPL_AUX_NA_SPEC2(2, 3, lambda)
#endif

}}

#endif // BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#endif // BOOST_MPL_AUX_FULL_LAMBDA_HPP_INCLUDED

///// iteration, depth == 1

// For gcc 4.4 compatability, we must include the
// BOOST_PP_ITERATION_DEPTH test inside an #else clause.
#else // BOOST_PP_IS_ITERATING
#if POTHOS_PP_ITERATION_DEPTH() == 1
#define i_ POTHOS_PP_FRAME_ITERATION(1)

#if i_ > 0

namespace aux {

#   define AUX778076_RESULT(unused, i_, T) \
    POTHOS_PP_COMMA_IF(i_) \
    typename POTHOS_PP_CAT(T, POTHOS_PP_INC(i_))::result_ \
    /**/

#   define AUX778076_TYPE(unused, i_, T) \
    POTHOS_PP_COMMA_IF(i_) \
    typename POTHOS_PP_CAT(T, POTHOS_PP_INC(i_))::type \
    /**/

template<
      typename IsLE, typename Tag
    , template< AUX778076_LAMBDA_PARAMS(i_, typename P) > class F
    , AUX778076_LAMBDA_PARAMS(i_, typename L)
    >
struct POTHOS_PP_CAT(le_result,i_)
{
    typedef F<
          POTHOS_MPL_PP_REPEAT(i_, AUX778076_TYPE, L)
        > result_;
    
    typedef result_ type;
};

template<
      typename Tag
    , template< AUX778076_LAMBDA_PARAMS(i_, typename P) > class F
    , AUX778076_LAMBDA_PARAMS(i_, typename L)
    >
struct POTHOS_PP_CAT(le_result,i_)< true_,Tag,F,AUX778076_LAMBDA_PARAMS(i_, L) >
{
    typedef POTHOS_PP_CAT(bind,i_)<
          POTHOS_PP_CAT(quote,i_)<F,Tag>
        , POTHOS_MPL_PP_REPEAT(i_, AUX778076_RESULT, L)
        > result_;

    typedef mpl::protect<result_> type;
};

#   undef AUX778076_TYPE
#   undef AUX778076_RESULT

} // namespace aux


#   define AUX778076_LAMBDA_TYPEDEF(unused, i_, T) \
    typedef lambda< POTHOS_PP_CAT(T, POTHOS_PP_INC(i_)), Tag > \
        POTHOS_PP_CAT(l,POTHOS_PP_INC(i_)); \
/**/

#   define AUX778076_IS_LE_TYPEDEF(unused, i_, unused2) \
    typedef typename POTHOS_PP_CAT(l,POTHOS_PP_INC(i_))::is_le \
        POTHOS_PP_CAT(is_le,POTHOS_PP_INC(i_)); \
/**/

#   define AUX778076_IS_LAMBDA_EXPR(unused, i_, unused2) \
    POTHOS_PP_COMMA_IF(i_) \
    POTHOS_PP_CAT(is_le,POTHOS_PP_INC(i_))::value \
/**/

template<
      template< AUX778076_LAMBDA_PARAMS(i_, typename P) > class F
    , AUX778076_LAMBDA_PARAMS(i_, typename T)
    , typename Tag
    >
struct lambda< 
          F<AUX778076_LAMBDA_PARAMS(i_, T)>
        , Tag
        AUX778076_ARITY_PARAM(int_<i_>)
        >
{
    POTHOS_MPL_PP_REPEAT(i_, AUX778076_LAMBDA_TYPEDEF, T)
    POTHOS_MPL_PP_REPEAT(i_, AUX778076_IS_LE_TYPEDEF, unused)

    typedef typename aux::lambda_or<
          POTHOS_MPL_PP_REPEAT(i_, AUX778076_IS_LAMBDA_EXPR, unused)
        >::type is_le;

    typedef aux::POTHOS_PP_CAT(le_result,i_)<
          is_le, Tag, F, AUX778076_LAMBDA_PARAMS(i_, l)
        > le_result_;
    
    typedef typename le_result_::result_ result_;
    typedef typename le_result_::type type;
};


#   undef AUX778076_IS_LAMBDA_EXPR
#   undef AUX778076_IS_LE_TYPEDEF
#   undef AUX778076_LAMBDA_TYPEDEF

#endif // i_ > 0

template<
      typename F AUX778076_BIND_N_PARAMS(i_, typename T)
    , typename Tag
    >
struct lambda<
          POTHOS_PP_CAT(bind,i_)<F AUX778076_BIND_N_PARAMS(i_, T)>
        , Tag
        AUX778076_ARITY_PARAM(int_<POTHOS_PP_INC(i_)>)
        >
{
    typedef false_ is_le;
    typedef POTHOS_PP_CAT(bind,i_)<
          F
        AUX778076_BIND_N_PARAMS(i_, T)
        > result_;
        
    typedef result_ type;
};

#undef i_
#endif // BOOST_PP_ITERATION_DEPTH()
#endif // BOOST_PP_IS_ITERATING
