
#if !defined(POTHOS_PP_IS_ITERATING)

///// header body

#ifndef POTHOS_MPL_AUX_LAMBDA_NO_CTPS_HPP_INCLUDED
#define POTHOS_MPL_AUX_LAMBDA_NO_CTPS_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: lambda_no_ctps.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#if !defined(POTHOS_MPL_PREPROCESSING_MODE)
#   include <Pothos/serialization/impl/mpl/lambda_fwd.hpp>
#   include <Pothos/serialization/impl/mpl/bind_fwd.hpp>
#   include <Pothos/serialization/impl/mpl/protect.hpp>
#   include <Pothos/serialization/impl/mpl/is_placeholder.hpp>
#   include <Pothos/serialization/impl/mpl/if.hpp>
#   include <Pothos/serialization/impl/mpl/identity.hpp>
#   include <Pothos/serialization/impl/mpl/bool.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/na_spec.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/lambda_support.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/template_arity.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/value_wknd.hpp>
#endif

#include <Pothos/serialization/impl/mpl/aux_/config/use_preprocessed.hpp>

#if    !defined(POTHOS_MPL_CFG_NO_PREPROCESSED_HEADERS) \
    && !defined(POTHOS_MPL_PREPROCESSING_MODE)

#   define POTHOS_MPL_PREPROCESSED_HEADER lambda_no_ctps.hpp
#   include <Pothos/serialization/impl/mpl/aux_/include_preprocessed.hpp>

#else

#   include <Pothos/serialization/impl/mpl/limits/arity.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/params.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/default_params.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/repeat.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/enum.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/msvc.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/workaround.hpp>

#   include <Pothos/serialization/impl/preprocessor/comma_if.hpp>
#   include <Pothos/serialization/impl/preprocessor/iterate.hpp>
#   include <Pothos/serialization/impl/preprocessor/inc.hpp>
#   include <Pothos/serialization/impl/preprocessor/cat.hpp>

namespace Pothos { namespace mpl {

#   define AUX778076_LAMBDA_PARAMS(i_, param) \
    POTHOS_MPL_PP_PARAMS(i_, param) \
    /**/

namespace aux {

#define n_ POTHOS_MPL_LIMIT_METAFUNCTION_ARITY
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
#undef n_

template< typename Arity > struct lambda_impl
{
    template< typename T, typename Tag, typename Protect > struct result_
    {
        typedef T type;
        typedef is_placeholder<T> is_le;
    };
};

#define POTHOS_PP_ITERATION_PARAMS_1 \
    (3,(1, POTHOS_MPL_LIMIT_METAFUNCTION_ARITY, <boost/mpl/aux_/lambda_no_ctps.hpp>))
#include POTHOS_PP_ITERATE()

} // namespace aux

template<
      typename T
    , typename Tag
    , typename Protect
    >
struct lambda
{
    /// Metafunction forwarding confuses MSVC 6.x
    typedef typename aux::template_arity<T>::type arity_;
    typedef typename aux::lambda_impl<arity_>
        ::template result_< T,Tag,Protect > l_;

    typedef typename l_::type type;
    typedef typename l_::is_le is_le;
    
    POTHOS_MPL_AUX_LAMBDA_SUPPORT(3, lambda, (T, Tag, Protect))
};

POTHOS_MPL_AUX_NA_SPEC2(1, 3, lambda)

template<
      typename T
    >
struct is_lambda_expression
    : lambda<T>::is_le
{
};

#   undef AUX778076_LAMBDA_PARAMS

}}

#endif // BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#endif // BOOST_MPL_AUX_LAMBDA_NO_CTPS_HPP_INCLUDED

///// iteration, depth == 1

#else

#define i_ POTHOS_PP_FRAME_ITERATION(1)

#   define AUX778076_LAMBDA_TYPEDEF(unused, i_, F) \
    typedef lambda< \
          typename F::POTHOS_PP_CAT(arg,POTHOS_PP_INC(i_)) \
        , Tag \
        , false_ \
        > POTHOS_PP_CAT(l,POTHOS_PP_INC(i_)); \
    /**/

#   define AUX778076_IS_LE_TYPEDEF(unused, i_, unused2) \
    typedef typename POTHOS_PP_CAT(l,POTHOS_PP_INC(i_))::is_le \
        POTHOS_PP_CAT(is_le,POTHOS_PP_INC(i_)); \
    /**/

#   define AUX778076_IS_LAMBDA_EXPR(unused, i_, unused2) \
    POTHOS_PP_COMMA_IF(i_) \
    POTHOS_MPL_AUX_MSVC_VALUE_WKND(POTHOS_PP_CAT(is_le,POTHOS_PP_INC(i_)))::value \
    /**/

#   define AUX778076_LAMBDA_RESULT(unused, i_, unused2) \
    , typename POTHOS_PP_CAT(l,POTHOS_PP_INC(i_))::type \
    /**/

template<> struct lambda_impl< int_<i_> >
{
    template< typename F, typename Tag, typename Protect > struct result_
    {
        POTHOS_MPL_PP_REPEAT(i_, AUX778076_LAMBDA_TYPEDEF, F)
        POTHOS_MPL_PP_REPEAT(i_, AUX778076_IS_LE_TYPEDEF, unused)

        typedef aux::lambda_or<
              POTHOS_MPL_PP_REPEAT(i_, AUX778076_IS_LAMBDA_EXPR, unused)
            > is_le;

        typedef POTHOS_PP_CAT(bind,i_)<
              typename F::rebind
            POTHOS_MPL_PP_REPEAT(i_, AUX778076_LAMBDA_RESULT, unused)
            > bind_;

        typedef typename if_<
              is_le
            , if_< Protect, mpl::protect<bind_>, bind_ >
            , identity<F>
            >::type type_;
    
        typedef typename type_::type type;
    };
};

#   undef AUX778076_LAMBDA_RESULT
#   undef AUX778076_IS_LAMBDA_EXPR
#   undef AUX778076_IS_LE_TYPEDEF
#   undef AUX778076_LAMBDA_TYPEDEF

#undef i_

#endif // BOOST_PP_IS_ITERATING
