
// NO INCLUDE GUARDS, THE HEADER IS INTENDED FOR MULTIPLE INCLUSION

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: shift_op.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#if !defined(POTHOS_MPL_PREPROCESSING_MODE)
#   include <Pothos/serialization/impl/mpl/integral_c.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/value_wknd.hpp>
#endif

#if !defined(AUX778076_OP_PREFIX)
#   define AUX778076_OP_PREFIX AUX778076_OP_NAME
#endif

#define AUX778076_OP_ARITY 2

#include <Pothos/serialization/impl/mpl/aux_/numeric_op.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/static_constant.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/use_preprocessed.hpp>

#if !defined(POTHOS_MPL_CFG_NO_PREPROCESSED_HEADERS) \
    && !defined(POTHOS_MPL_PREPROCESSING_MODE)

#   define POTHOS_MPL_PREPROCESSED_HEADER AUX778076_OP_PREFIX.hpp
#   include <Pothos/serialization/impl/mpl/aux_/include_preprocessed.hpp>

#else

#   include <Pothos/serialization/impl/mpl/aux_/config/integral.hpp>
#   include <Pothos/serialization/impl/preprocessor/cat.hpp>

namespace Pothos { namespace mpl {

#if defined(POTHOS_MPL_CFG_NO_NESTED_VALUE_ARITHMETIC)
namespace aux {
template< typename T, typename Shift, T n, Shift s >
struct POTHOS_PP_CAT(AUX778076_OP_PREFIX,_wknd)
{
    POTHOS_STATIC_CONSTANT(T, value = (n AUX778076_OP_TOKEN s));
    typedef integral_c<T,value> type;
};
}
#endif

template<>
struct AUX778076_OP_IMPL_NAME<integral_c_tag,integral_c_tag>
{
    template< typename N, typename S > struct apply
#if !defined(POTHOS_MPL_CFG_NO_NESTED_VALUE_ARITHMETIC)
        : integral_c<
              typename N::value_type
            , ( POTHOS_MPL_AUX_VALUE_WKND(N)::value
                  AUX778076_OP_TOKEN POTHOS_MPL_AUX_VALUE_WKND(S)::value
                )
            >
#else
        : aux::POTHOS_PP_CAT(AUX778076_OP_PREFIX,_wknd)<
              typename N::value_type
            , typename S::value_type
            , N::value
            , S::value
            >::type
#endif
    {
    };
};

}}

#endif // BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS

#undef AUX778076_OP_TAG_NAME
#undef AUX778076_OP_IMPL_NAME
#undef AUX778076_OP_ARITY
#undef AUX778076_OP_PREFIX
#undef AUX778076_OP_NAME
#undef AUX778076_OP_TOKEN
