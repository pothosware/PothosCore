
#if !defined(POTHOS_PP_IS_ITERATING)

///// header body

#ifndef POTHOS_MPL_QUOTE_HPP_INCLUDED
#define POTHOS_MPL_QUOTE_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2008
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: quote.hpp 49272 2008-10-11 06:50:46Z agurtovoy $
// $Date: 2008-10-10 23:50:46 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49272 $

#if !defined(POTHOS_MPL_PREPROCESSING_MODE)
#   include <Pothos/serialization/impl/mpl/void.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/has_type.hpp>
#endif

#include <Pothos/serialization/impl/mpl/aux_/config/bcc.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/ttp.hpp>

#if defined(POTHOS_MPL_CFG_NO_TEMPLATE_TEMPLATE_PARAMETERS) \
    && !defined(POTHOS_MPL_CFG_BCC590_WORKAROUNDS)
#   define POTHOS_MPL_CFG_NO_QUOTE_TEMPLATE
#endif

#if !defined(POTHOS_MPL_CFG_NO_IMPLICIT_METAFUNCTIONS) \
    && defined(POTHOS_MPL_CFG_NO_HAS_XXX)
#   define POTHOS_MPL_CFG_NO_IMPLICIT_METAFUNCTIONS
#endif

#include <Pothos/serialization/impl/mpl/aux_/config/use_preprocessed.hpp>

#if !defined(POTHOS_MPL_CFG_NO_PREPROCESSED_HEADERS) \
 && !defined(POTHOS_MPL_PREPROCESSING_MODE)

#   define POTHOS_MPL_PREPROCESSED_HEADER quote.hpp
#   include <Pothos/serialization/impl/mpl/aux_/include_preprocessed.hpp>

#else

#   include <Pothos/serialization/impl/mpl/limits/arity.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/params.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/ctps.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/workaround.hpp>

#   include <Pothos/serialization/impl/preprocessor/iterate.hpp>
#   include <Pothos/serialization/impl/preprocessor/cat.hpp>

#if !defined(POTHOS_MPL_CFG_NO_QUOTE_TEMPLATE)

namespace Pothos { namespace mpl {

#if !defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

template< typename T, bool has_type_ >
struct quote_impl
// GCC has a problem with metafunction forwarding when T is a
// specialization of a template called 'type'.
# if POTHOS_WORKAROUND(__GNUC__, POTHOS_TESTED_AT(4)) \
    && POTHOS_WORKAROUND(__GNUC_MINOR__, POTHOS_TESTED_AT(0)) \
    && POTHOS_WORKAROUND(__GNUC_PATCHLEVEL__, POTHOS_TESTED_AT(2))
{
    typedef typename T::type type;
};
# else 
    : T
{
};
# endif 

template< typename T >
struct quote_impl<T,false>
{
    typedef T type;
};

#else // BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

template< bool > struct quote_impl
{
    template< typename T > struct result_
        : T
    {
    };
};

template<> struct quote_impl<false>
{
    template< typename T > struct result_
    {
        typedef T type;
    };
};

#endif 

#define POTHOS_PP_ITERATION_PARAMS_1 \
    (3,(1, POTHOS_MPL_LIMIT_METAFUNCTION_ARITY, <boost/mpl/quote.hpp>))
#include POTHOS_PP_ITERATE()

}}

#endif // BOOST_MPL_CFG_NO_QUOTE_TEMPLATE

#endif // BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#endif // BOOST_MPL_QUOTE_HPP_INCLUDED

///// iteration

#else
#define i_ POTHOS_PP_FRAME_ITERATION(1)

template<
      template< POTHOS_MPL_PP_PARAMS(i_, typename P) > class F
    , typename Tag = void_
    >
struct POTHOS_PP_CAT(quote,i_)
{
    template< POTHOS_MPL_PP_PARAMS(i_, typename U) > struct apply
#if defined(POTHOS_MPL_CFG_BCC590_WORKAROUNDS)
    {
        typedef typename quote_impl<
              F< POTHOS_MPL_PP_PARAMS(i_, U) >
            , aux::has_type< F< POTHOS_MPL_PP_PARAMS(i_, U) > >::value
            >::type type;
    };
#elif !defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
        : quote_impl<
              F< POTHOS_MPL_PP_PARAMS(i_, U) >
            , aux::has_type< F< POTHOS_MPL_PP_PARAMS(i_, U) > >::value
            >
    {
    };
#else
        : quote_impl< aux::has_type< F< POTHOS_MPL_PP_PARAMS(i_, U) > >::value >
            ::template result_< F< POTHOS_MPL_PP_PARAMS(i_, U) > >
    {
    };
#endif
};

#undef i_
#endif // BOOST_PP_IS_ITERATING
