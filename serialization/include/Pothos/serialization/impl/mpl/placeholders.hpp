
#if !defined(POTHOS_PP_IS_ITERATING)

///// header body

#ifndef POTHOS_MPL_PLACEHOLDERS_HPP_INCLUDED
#define POTHOS_MPL_PLACEHOLDERS_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2001-2004
// Copyright Peter Dimov 2001-2003
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: placeholders.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $


#if !defined(POTHOS_MPL_PREPROCESSING_MODE)
#   include <Pothos/serialization/impl/mpl/arg.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/adl_barrier.hpp>

#   if !defined(POTHOS_MPL_CFG_NO_ADL_BARRIER_NAMESPACE)
#       define POTHOS_MPL_AUX_ARG_ADL_BARRIER_DECL(type) \
        using ::POTHOS_MPL_AUX_ADL_BARRIER_NAMESPACE::type; \
        /**/
#   else
#       define POTHOS_MPL_AUX_ARG_ADL_BARRIER_DECL(type) /**/
#   endif

#endif

#include <Pothos/serialization/impl/mpl/aux_/config/use_preprocessed.hpp>

#if !defined(POTHOS_MPL_CFG_NO_PREPROCESSED_HEADERS) \
 && !defined(POTHOS_MPL_PREPROCESSING_MODE)

#   define POTHOS_MPL_PREPROCESSED_HEADER placeholders.hpp
#   include <Pothos/serialization/impl/mpl/aux_/include_preprocessed.hpp>

#else

#   include <Pothos/serialization/impl/mpl/aux_/nttp_decl.hpp>
#   include <Pothos/serialization/impl/mpl/limits/arity.hpp>
#   include <Pothos/serialization/impl/preprocessor/iterate.hpp>
#   include <Pothos/serialization/impl/preprocessor/cat.hpp>

// watch out for GNU gettext users, who #define _(x)
#if !defined(_) || defined(POTHOS_MPL_CFG_NO_UNNAMED_PLACEHOLDER_SUPPORT)
POTHOS_MPL_AUX_ADL_BARRIER_NAMESPACE_OPEN
typedef arg<-1> _;
POTHOS_MPL_AUX_ADL_BARRIER_NAMESPACE_CLOSE

namespace Pothos { namespace mpl { 

POTHOS_MPL_AUX_ARG_ADL_BARRIER_DECL(_)

namespace placeholders {
using POTHOS_MPL_AUX_ADL_BARRIER_NAMESPACE::_;
}

}}
#endif

/// agurt, 17/mar/02: one more placeholder for the last 'apply#' 
/// specialization
#define POTHOS_PP_ITERATION_PARAMS_1 \
    (3,(1, POTHOS_MPL_LIMIT_METAFUNCTION_ARITY + 1, <boost/mpl/placeholders.hpp>))
#include POTHOS_PP_ITERATE()

#endif // BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#endif // BOOST_MPL_PLACEHOLDERS_HPP_INCLUDED

///// iteration

#else
#define i_ POTHOS_PP_FRAME_ITERATION(1)

POTHOS_MPL_AUX_ADL_BARRIER_NAMESPACE_OPEN

typedef arg<i_> POTHOS_PP_CAT(_,i_);

POTHOS_MPL_AUX_ADL_BARRIER_NAMESPACE_CLOSE

namespace Pothos { namespace mpl { 

POTHOS_MPL_AUX_ARG_ADL_BARRIER_DECL(POTHOS_PP_CAT(_,i_))

namespace placeholders {
using POTHOS_MPL_AUX_ADL_BARRIER_NAMESPACE::POTHOS_PP_CAT(_,i_);
}

}}

#undef i_
#endif // BOOST_PP_IS_ITERATING
