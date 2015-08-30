
// NO INCLUDE GUARDS, THE HEADER IS INTENDED FOR MULTIPLE INCLUSION

// Copyright Aleksey Gurtovoy 2002-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)

#include <Pothos/serialization/impl/mpl/int.hpp>
#include <Pothos/serialization/impl/mpl/aux_/template_arity_fwd.hpp>
#include <Pothos/serialization/impl/mpl/aux_/preprocessor/params.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/lambda.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/overload_resolution.hpp>

#if defined(POTHOS_MPL_CFG_NO_FULL_LAMBDA_SUPPORT) \
    && defined(POTHOS_MPL_CFG_BROKEN_OVERLOAD_RESOLUTION)
#   define POTHOS_TT_AUX_TEMPLATE_ARITY_SPEC(i, name) \
namespace mpl { namespace aux { \
template< POTHOS_MPL_PP_PARAMS(i, typename T) > \
struct template_arity< \
          name< POTHOS_MPL_PP_PARAMS(i, T) > \
        > \
    : int_<i> \
{ \
}; \
}} \
/**/
#else
#   define POTHOS_TT_AUX_TEMPLATE_ARITY_SPEC(i, name) /**/
#endif
