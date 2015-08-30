
#if !defined(POTHOS_PP_IS_ITERATING)

///// header body

#ifndef POTHOS_MPL_INHERIT_HPP_INCLUDED
#define POTHOS_MPL_INHERIT_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2001-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: inherit.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#if !defined(POTHOS_MPL_PREPROCESSING_MODE)
#   include <Pothos/serialization/impl/mpl/empty_base.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/na_spec.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/lambda_support.hpp>
#endif

#include <Pothos/serialization/impl/mpl/aux_/config/use_preprocessed.hpp>

#if !defined(POTHOS_MPL_CFG_NO_PREPROCESSED_HEADERS) \
    && !defined(POTHOS_MPL_PREPROCESSING_MODE)

#   define POTHOS_MPL_PREPROCESSED_HEADER inherit.hpp
#   include <Pothos/serialization/impl/mpl/aux_/include_preprocessed.hpp>

#else

#   include <Pothos/serialization/impl/mpl/limits/arity.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/params.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/default_params.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/enum.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/ctps.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/dtp.hpp>

#   include <Pothos/serialization/impl/preprocessor/iterate.hpp>
#   include <Pothos/serialization/impl/preprocessor/dec.hpp>
#   include <Pothos/serialization/impl/preprocessor/cat.hpp>

namespace Pothos { namespace mpl {

// 'inherit<T1,T2,..,Tn>' metafunction; returns an unspecified class type
// produced by public derivation from all metafunction's parameters 
// (T1,T2,..,Tn), except the parameters of 'empty_base' class type; 
// regardless the position and number of 'empty_base' parameters in the 
// metafunction's argument list, derivation from them is always a no-op;
// for instance:
//      inherit<her>::type == her
//      inherit<her,my>::type == struct unspecified : her, my {};
//      inherit<empty_base,her>::type == her
//      inherit<empty_base,her,empty_base,empty_base>::type == her
//      inherit<her,empty_base,my>::type == struct unspecified : her, my {};
//      inherit<empty_base,empty_base>::type == empty_base

#if !defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

template< 
      typename POTHOS_MPL_AUX_NA_PARAM(T1)
    , typename POTHOS_MPL_AUX_NA_PARAM(T2)
    > 
struct inherit2
    : T1, T2
{
    typedef inherit2 type;
    POTHOS_MPL_AUX_LAMBDA_SUPPORT(2, inherit2, (T1,T2))
};

template< typename T1 > 
struct inherit2<T1,empty_base>
{
    typedef T1 type;
    POTHOS_MPL_AUX_LAMBDA_SUPPORT_SPEC(2, inherit2, (T1,empty_base))
};

template< typename T2 > 
struct inherit2<empty_base,T2>
{
    typedef T2 type;
    POTHOS_MPL_AUX_LAMBDA_SUPPORT_SPEC(2, inherit2, (empty_base,T2))
};

// needed to disambiguate the previous two in case when both 
// T1 and T2 == empty_base
template<> 
struct inherit2<empty_base,empty_base>
{
    typedef empty_base type;
    POTHOS_MPL_AUX_LAMBDA_SUPPORT_SPEC(2, inherit2, (empty_base,empty_base))
};

#else

namespace aux {

template< bool C1, bool C2 >
struct inherit2_impl
{
    template< typename Derived, typename T1, typename T2 > struct result_ 
        : T1, T2
    {
        typedef Derived type_;
    };
};

template<>
struct inherit2_impl<false,true>
{
    template< typename Derived, typename T1, typename T2 > struct result_
        : T1
    {
        typedef T1 type_;
    };
};

template<>
struct inherit2_impl<true,false>
{
    template< typename Derived, typename T1, typename T2 > struct result_
        : T2 
    {
        typedef T2 type_;
    };
};

template<>
struct inherit2_impl<true,true>
{
    template< typename Derived, typename T1, typename T2 > struct result_
    {
        typedef T1 type_;
    };
};

} // namespace aux

template< 
      typename POTHOS_MPL_AUX_NA_PARAM(T1)
    , typename POTHOS_MPL_AUX_NA_PARAM(T2)
    > 
struct inherit2
    : aux::inherit2_impl<
          is_empty_base<T1>::value
        , is_empty_base<T2>::value
        >::template result_< inherit2<T1,T2>,T1,T2 >
{
    typedef typename inherit2::type_ type;
    POTHOS_MPL_AUX_LAMBDA_SUPPORT(2, inherit2, (T1,T2))
};

#endif // BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

POTHOS_MPL_AUX_NA_SPEC(2, inherit2)

#define POTHOS_PP_ITERATION_PARAMS_1 \
    (3,(3, POTHOS_MPL_LIMIT_METAFUNCTION_ARITY, <boost/mpl/inherit.hpp>))
#include POTHOS_PP_ITERATE()

}}

#endif // BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#endif // BOOST_MPL_INHERIT_HPP_INCLUDED

///// iteration

#else
#define n_ POTHOS_PP_FRAME_ITERATION(1)

template<
      POTHOS_MPL_PP_DEFAULT_PARAMS(n_, typename T, na)
    >
struct POTHOS_PP_CAT(inherit,n_)
    : inherit2<
          typename POTHOS_PP_CAT(inherit,POTHOS_PP_DEC(n_))<
              POTHOS_MPL_PP_PARAMS(POTHOS_PP_DEC(n_), T)
            >::type
        , POTHOS_PP_CAT(T,n_)
        >
{
    POTHOS_MPL_AUX_LAMBDA_SUPPORT(
          n_
        , POTHOS_PP_CAT(inherit,n_)
        , (POTHOS_MPL_PP_PARAMS(n_, T))
        )
};

POTHOS_MPL_AUX_NA_SPEC(n_, POTHOS_PP_CAT(inherit,n_))

#if n_ == POTHOS_MPL_LIMIT_METAFUNCTION_ARITY
/// primary template
template<
      POTHOS_MPL_PP_DEFAULT_PARAMS(n_, typename T, empty_base)
    >
struct inherit
    : POTHOS_PP_CAT(inherit,n_)<POTHOS_MPL_PP_PARAMS(n_, T)>
{
};

// 'na' specialization
template<>
struct inherit< POTHOS_MPL_PP_ENUM(5, na) >
{
    template<
#if !defined(POTHOS_MPL_CFG_NO_DEFAULT_PARAMETERS_IN_NESTED_TEMPLATES)
          POTHOS_MPL_PP_DEFAULT_PARAMS(n_, typename T, empty_base)
#else
          POTHOS_MPL_PP_PARAMS(n_, typename T)
#endif
        >
    struct apply
        : inherit< POTHOS_MPL_PP_PARAMS(n_, T) >
    {
    };
};

POTHOS_MPL_AUX_NA_SPEC_LAMBDA(n_, inherit)
POTHOS_MPL_AUX_NA_SPEC_ARITY(n_, inherit)
POTHOS_MPL_AUX_NA_SPEC_TEMPLATE_ARITY(n_, n_, inherit)
#endif

#undef n_
#endif // BOOST_PP_IS_ITERATING
