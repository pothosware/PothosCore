
#if !defined(POTHOS_PP_IS_ITERATING)

///// header body

#ifndef POTHOS_MPL_UNPACK_ARGS_HPP_INCLUDED
#define POTHOS_MPL_UNPACK_ARGS_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2002-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: unpack_args.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#if !defined(POTHOS_MPL_PREPROCESSING_MODE)
#   include <Pothos/serialization/impl/mpl/apply.hpp>
#   include <Pothos/serialization/impl/mpl/at.hpp>
#   include <Pothos/serialization/impl/mpl/size.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/nttp_decl.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/lambda_spec.hpp>
#endif

#include <Pothos/serialization/impl/mpl/aux_/config/use_preprocessed.hpp>

#if !defined(POTHOS_MPL_CFG_NO_PREPROCESSED_HEADERS) \
    && !defined(POTHOS_MPL_PREPROCESSING_MODE)

#   define POTHOS_MPL_PREPROCESSED_HEADER unpack_args.hpp
#   include <Pothos/serialization/impl/mpl/aux_/include_preprocessed.hpp>

#else

#   include <Pothos/serialization/impl/mpl/limits/arity.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/repeat.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/ctps.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/forwarding.hpp>
#   include <Pothos/serialization/impl/preprocessor/iterate.hpp>
#   include <Pothos/serialization/impl/preprocessor/cat.hpp>


namespace Pothos { namespace mpl {

// local macros, #undef-ined at the end of the header

#   define AUX778076_UNPACK(unused, i, Args) \
    , typename at_c<Args,i>::type \
    /**/

#   define AUX778076_UNPACKED_ARGS(n, Args) \
    POTHOS_MPL_PP_REPEAT(n, AUX778076_UNPACK, Args) \
    /**/

namespace aux {

#if !defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
template< int size, typename F, typename Args >
struct unpack_args_impl;
#else
template< POTHOS_MPL_AUX_NTTP_DECL(int, size) > struct unpack_args_impl
{
    template< typename F, typename Args > struct apply;
};
#endif

#define POTHOS_PP_ITERATION_PARAMS_1 \
    (3,(0, POTHOS_MPL_LIMIT_METAFUNCTION_ARITY, <boost/mpl/unpack_args.hpp>))
#include POTHOS_PP_ITERATE()

}

template<
      typename F
    >
struct unpack_args
{
    template< typename Args > struct apply
#if !defined(POTHOS_MPL_CFG_NO_NESTED_FORWARDING)
#   if !defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
        : aux::unpack_args_impl< size<Args>::value,F,Args >
#   else
        : aux::unpack_args_impl< size<Args>::value >
            ::template apply< F,Args >
#   endif
    {
#else // BOOST_MPL_CFG_NO_NESTED_FORWARDING
    {
        typedef typename aux::unpack_args_impl< 
              size<Args>::value
            , F
            , Args
            >::type type;
#endif
    };
};

POTHOS_MPL_AUX_PASS_THROUGH_LAMBDA_SPEC(1, unpack_args)

#   undef AUX778076_UNPACK
#   undef AUX778076_UNPACKED_ARGS

}}

#endif // BOOST_MPL_CFG_USE_PREPROCESSED_HEADERS
#endif // BOOST_MPL_UNPACK_ARGS_HPP_INCLUDED

///// iteration, depth == 1

// For gcc 4.4 compatability, we must include the
// BOOST_PP_ITERATION_DEPTH test inside an #else clause.
#else // BOOST_PP_IS_ITERATING
#if POTHOS_PP_ITERATION_DEPTH() == 1

#   define i_ POTHOS_PP_FRAME_ITERATION(1)

#if !defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

template< typename F, typename Args >
struct unpack_args_impl<i_,F,Args>
    : POTHOS_PP_CAT(apply,i_)<
          F
        AUX778076_UNPACKED_ARGS(i_, Args)
        >
{
};

#else

template<> struct unpack_args_impl<i_>
{
    template< typename F, typename Args > struct apply
        : POTHOS_PP_CAT(apply,i_)<
              F
            AUX778076_UNPACKED_ARGS(i_, Args)
            >
    {
    };
};

#endif

#   undef i_

#endif // BOOST_PP_ITERATION_DEPTH()
#endif // BOOST_PP_IS_ITERATING
