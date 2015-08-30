
#ifndef POTHOS_MPL_VECTOR_VECTOR50_HPP_INCLUDED
#define POTHOS_MPL_VECTOR_VECTOR50_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: vector50.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#if !defined(POTHOS_MPL_PREPROCESSING_MODE)
#   include <Pothos/serialization/impl/mpl/vector/vector40.hpp>
#endif

#include <Pothos/serialization/impl/mpl/aux_/config/use_preprocessed.hpp>

#if !defined(POTHOS_MPL_CFG_NO_PREPROCESSED_HEADERS) \
    && !defined(POTHOS_MPL_PREPROCESSING_MODE)

#   define POTHOS_MPL_PREPROCESSED_HEADER vector50.hpp
#   include <Pothos/serialization/impl/mpl/vector/aux_/include_preprocessed.hpp>

#else

#   include <Pothos/serialization/impl/mpl/aux_/config/typeof.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/ctps.hpp>
#   include <Pothos/serialization/impl/preprocessor/iterate.hpp>

namespace Pothos { namespace mpl {

#   define POTHOS_PP_ITERATION_PARAMS_1 \
    (3,(41, 50, <boost/mpl/vector/aux_/numbered.hpp>))
#   include POTHOS_PP_ITERATE()

}}

#endif // BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS

#endif // BOOST_MPL_VECTOR_VECTOR50_HPP_INCLUDED
