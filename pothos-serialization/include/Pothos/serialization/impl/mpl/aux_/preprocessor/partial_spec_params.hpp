
#ifndef POTHOS_MPL_AUX_PREPROCESSOR_PARTIAL_SPEC_PARAMS_HPP_INCLUDED
#define POTHOS_MPL_AUX_PREPROCESSOR_PARTIAL_SPEC_PARAMS_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: partial_spec_params.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/limits/arity.hpp>
#include <Pothos/serialization/impl/mpl/aux_/preprocessor/params.hpp>
#include <Pothos/serialization/impl/mpl/aux_/preprocessor/enum.hpp>
#include <Pothos/serialization/impl/mpl/aux_/preprocessor/sub.hpp>
#include <Pothos/serialization/impl/preprocessor/comma_if.hpp>

#define POTHOS_MPL_PP_PARTIAL_SPEC_PARAMS(n, param, def) \
POTHOS_MPL_PP_PARAMS(n, param) \
POTHOS_PP_COMMA_IF(POTHOS_MPL_PP_SUB(POTHOS_MPL_LIMIT_METAFUNCTION_ARITY,n)) \
POTHOS_MPL_PP_ENUM( \
      POTHOS_MPL_PP_SUB(POTHOS_MPL_LIMIT_METAFUNCTION_ARITY,n) \
    , def \
    ) \
/**/

#endif // BOOST_MPL_AUX_PREPROCESSOR_PARTIAL_SPEC_PARAMS_HPP_INCLUDED
