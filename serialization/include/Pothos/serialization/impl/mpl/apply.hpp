
#if !defined(POTHOS_PP_IS_ITERATING)

///// header body

#ifndef POTHOS_MPL_APPLY_HPP_INCLUDED
#define POTHOS_MPL_APPLY_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: apply.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#if !defined(POTHOS_MPL_PREPROCESSING_MODE)
#   include <Pothos/serialization/impl/mpl/apply_fwd.hpp>
#   include <Pothos/serialization/impl/mpl/apply_wrap.hpp>
#   include <Pothos/serialization/impl/mpl/placeholders.hpp>
#   include <Pothos/serialization/impl/mpl/lambda.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/na.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/lambda_support.hpp>
#endif

#include <Pothos/serialization/impl/mpl/aux_/config/use_preprocessed.hpp>

#if !defined(POTHOS_MPL_CFG_NO_PREPROCESSED_HEADERS) \
    && !defined(POTHOS_MPL_PREPROCESSING_MODE)

#   define POTHOS_MPL_PREPROCESSED_HEADER apply.hpp
#   include <Pothos/serialization/impl/mpl/aux_/include_preprocessed.hpp>

#else

#   include <Pothos/serialization/impl/mpl/limits/arity.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/params.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/default_params.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/partial_spec_params.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/enum.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/lambda.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/dtp.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/nttp_decl.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/eti.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/msvc.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/workaround.hpp>

#   include <Pothos/serialization/impl/preprocessor/comma_if.hpp>
#   include <Pothos/serialization/impl/preprocessor/inc.hpp>
#   include <Pothos/serialization/impl/preprocessor/iterate.hpp>
#   include <Pothos/serialization/impl/preprocessor/cat.hpp>

namespace Pothos { namespace mpl {

// local macros, #undef-ined at the end of the header
#   define AUX778076_APPLY_PARAMS(param) \
    POTHOS_MPL_PP_PARAMS( \
          POTHOS_MPL_LIMIT_METAFUNCTION_ARITY \
        , param \
        ) \
    /**/

#   define AUX778076_APPLY_DEF_PARAMS(param, value) \
    POTHOS_MPL_PP_DEFAULT_PARAMS( \
          POTHOS_MPL_LIMIT_METAFUNCTION_ARITY \
        , param \
        , value \
        ) \
    /**/

#   define AUX778076_APPLY_N_PARAMS(n, param) \
    POTHOS_MPL_PP_PARAMS(n, param) \
    /**/

#   define AUX778076_APPLY_N_COMMA_PARAMS(n, param) \
    POTHOS_PP_COMMA_IF(n) \
    POTHOS_MPL_PP_PARAMS(n, param) \
    /**/

#   define AUX778076_APPLY_N_PARTIAL_SPEC_PARAMS(n, param, def) \
    POTHOS_PP_COMMA_IF(n) \
    POTHOS_MPL_PP_PARTIAL_SPEC_PARAMS(n, param, def) \
    /**/
    
#   define AUX778076_APPLY_N_SPEC_PARAMS(n, param) \
    POTHOS_MPL_PP_ENUM(POTHOS_PP_INC(n), param) \
    /**/


#define POTHOS_PP_ITERATION_PARAMS_1 \
    (3,(0, POTHOS_MPL_LIMIT_METAFUNCTION_ARITY, <boost/mpl/apply.hpp>))
#include POTHOS_PP_ITERATE()

#   if !defined(POTHOS_MPL_CFG_NO_APPLY_TEMPLATE)
// real C++ version is already taken care of
#   if defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

namespace aux {
// apply_count_args
#define AUX778076_COUNT_ARGS_PREFIX apply
#define AUX778076_COUNT_ARGS_DEFAULT na
#define AUX778076_COUNT_ARGS_ARITY POTHOS_MPL_LIMIT_METAFUNCTION_ARITY
#include <Pothos/serialization/impl/mpl/aux_/count_args.hpp>
}


template<
      typename F, AUX778076_APPLY_DEF_PARAMS(typename T, na)
    >
struct apply
    : aux::apply_chooser< 
          aux::apply_count_args< AUX778076_APPLY_PARAMS(T) >::value
        >::template result_< F, AUX778076_APPLY_PARAMS(T) >::type
{
};

#   endif // BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
#   endif // BOOST_MPL_CFG_NO_APPLY_TEMPLATE

#   undef AUX778076_APPLY_N_SPEC_PARAMS
#   undef AUX778076_APPLY_N_PARTIAL_SPEC_PARAMS
#   undef AUX778076_APPLY_N_COMMA_PARAMS
#   undef AUX778076_APPLY_N_PARAMS
#   undef AUX778076_APPLY_DEF_PARAMS
#   undef AUX778076_APPLY_PARAMS

}}

#endif // BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#endif // BOOST_MPL_APPLY_HPP_INCLUDED

///// iteration, depth == 1

// For gcc 4.4 compatability, we must include the
// BOOST_PP_ITERATION_DEPTH test inside an #else clause.
#else // BOOST_PP_IS_ITERATING
#if POTHOS_PP_ITERATION_DEPTH() == 1

#   define i_ POTHOS_PP_FRAME_ITERATION(1)

template<
      typename F AUX778076_APPLY_N_COMMA_PARAMS(i_, typename T)
    >
struct POTHOS_PP_CAT(apply,i_)
#if !POTHOS_WORKAROUND(POTHOS_MSVC, < 1300)
    : POTHOS_PP_CAT(apply_wrap,i_)< 
          typename lambda<F>::type
        AUX778076_APPLY_N_COMMA_PARAMS(i_, T)
        >
{
#else
{
    typedef typename POTHOS_PP_CAT(apply_wrap,i_)< 
          typename lambda<F>::type
        AUX778076_APPLY_N_COMMA_PARAMS(i_, T)
        >::type type;
#endif
    POTHOS_MPL_AUX_LAMBDA_SUPPORT(
          POTHOS_PP_INC(i_)
        , POTHOS_PP_CAT(apply,i_)
        , (F AUX778076_APPLY_N_COMMA_PARAMS(i_,T))
        )
};


#if defined(POTHOS_MPL_CFG_MSVC_ETI_BUG)
/// workaround for ETI bug
template<>
struct POTHOS_PP_CAT(apply,i_)<AUX778076_APPLY_N_SPEC_PARAMS(i_, int)>
{
    typedef int type;
};
#endif

#   if !defined(POTHOS_MPL_CFG_NO_APPLY_TEMPLATE)
#   if !defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

#if i_ == POTHOS_MPL_LIMIT_METAFUNCTION_ARITY
/// primary template (not a specialization!)
template<
      typename F AUX778076_APPLY_N_COMMA_PARAMS(i_, typename T)
    >
struct apply
    : POTHOS_PP_CAT(apply,i_)< F AUX778076_APPLY_N_COMMA_PARAMS(i_, T) >
{
};
#else
template<
      typename F AUX778076_APPLY_N_COMMA_PARAMS(i_, typename T)
    >
struct apply< F AUX778076_APPLY_N_PARTIAL_SPEC_PARAMS(i_, T, na) >
    : POTHOS_PP_CAT(apply,i_)< F AUX778076_APPLY_N_COMMA_PARAMS(i_, T) >
{
};
#endif

#   else // BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

#if !defined(POTHOS_MPL_CFG_NO_APPLY_TEMPLATE)
namespace aux {

template<>
struct apply_chooser<i_>
{
    template<
          typename F, AUX778076_APPLY_PARAMS(typename T)
        >
    struct result_
    {
        typedef POTHOS_PP_CAT(apply,i_)<
              F AUX778076_APPLY_N_COMMA_PARAMS(i_, T)
            > type;
    };
};

} // namespace aux
#endif

#   endif // BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
#   endif // BOOST_MPL_CFG_NO_APPLY_TEMPLATE

#   undef i_

#endif // BOOST_PP_ITERATION_DEPTH()
#endif // BOOST_PP_IS_ITERATING
