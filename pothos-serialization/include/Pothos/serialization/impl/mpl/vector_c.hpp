
#ifndef POTHOS_MPL_VECTOR_C_HPP_INCLUDED
#define POTHOS_MPL_VECTOR_C_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2008
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: vector_c.hpp 49271 2008-10-11 06:46:00Z agurtovoy $
// $Date: 2008-10-10 23:46:00 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49271 $

#if !defined(POTHOS_MPL_PREPROCESSING_MODE)
#   include <Pothos/serialization/impl/mpl/limits/vector.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/nttp_decl.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/preprocessor.hpp>

#   include <Pothos/serialization/impl/preprocessor/inc.hpp>
#   include <Pothos/serialization/impl/preprocessor/cat.hpp>
#   include <Pothos/serialization/impl/preprocessor/stringize.hpp>

#if !defined(POTHOS_NEEDS_TOKEN_PASTING_OP_FOR_TOKENS_JUXTAPOSING)
#   define AUX778076_VECTOR_C_HEADER \
    POTHOS_PP_CAT(POTHOS_PP_CAT(vector,POTHOS_MPL_LIMIT_VECTOR_SIZE),_c).hpp \
    /**/
#else
#   define AUX778076_VECTOR_C_HEADER \
    POTHOS_PP_CAT(POTHOS_PP_CAT(vector,POTHOS_MPL_LIMIT_VECTOR_SIZE),_c)##.hpp \
    /**/
#endif

#   include POTHOS_PP_STRINGIZE(Pothos/serialization/impl/mpl/vector/AUX778076_VECTOR_C_HEADER)
#   undef AUX778076_VECTOR_C_HEADER
#   include <climits>
#endif

#include <Pothos/serialization/impl/mpl/aux_/config/use_preprocessed.hpp>

#if !defined(POTHOS_MPL_CFG_NO_PREPROCESSED_HEADERS) \
    && !defined(POTHOS_MPL_PREPROCESSING_MODE)

#   define POTHOS_MPL_PREPROCESSED_HEADER vector_c.hpp
#   include <Pothos/serialization/impl/mpl/aux_/include_preprocessed.hpp>

#else

#   include <Pothos/serialization/impl/mpl/limits/vector.hpp>

#   define AUX778076_SEQUENCE_NAME vector_c
#   define AUX778076_SEQUENCE_LIMIT POTHOS_MPL_LIMIT_VECTOR_SIZE
#   define AUX778076_SEQUENCE_NAME_N(n) POTHOS_PP_CAT(POTHOS_PP_CAT(vector,n),_c)
#   define AUX778076_SEQUENCE_CONVERT_CN_TO(z,n,TARGET) TARGET(POTHOS_PP_CAT(C,n))
#   define AUX778076_SEQUENCE_INTEGRAL_WRAPPER
#   include <Pothos/serialization/impl/mpl/aux_/sequence_wrapper.hpp>

#endif // BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#endif // BOOST_MPL_VECTOR_C_HPP_INCLUDED
