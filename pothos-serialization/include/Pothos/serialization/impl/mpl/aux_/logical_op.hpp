
// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: logical_op.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

// NO INCLUDE GUARDS, THE HEADER IS INTENDED FOR MULTIPLE INCLUSION!

#if !defined(POTHOS_MPL_PREPROCESSING_MODE)
#   include <Pothos/serialization/impl/mpl/bool.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/nested_type_wknd.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/na_spec.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/lambda_support.hpp>
#endif

#include <Pothos/serialization/impl/mpl/limits/arity.hpp>
#include <Pothos/serialization/impl/mpl/aux_/preprocessor/params.hpp>
#include <Pothos/serialization/impl/mpl/aux_/preprocessor/ext_params.hpp>
#include <Pothos/serialization/impl/mpl/aux_/preprocessor/def_params_tail.hpp>
#include <Pothos/serialization/impl/mpl/aux_/preprocessor/enum.hpp>
#include <Pothos/serialization/impl/mpl/aux_/preprocessor/sub.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/ctps.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/workaround.hpp>

#include <Pothos/serialization/impl/preprocessor/dec.hpp>
#include <Pothos/serialization/impl/preprocessor/inc.hpp>
#include <Pothos/serialization/impl/preprocessor/cat.hpp>

namespace Pothos { namespace mpl {

#   define AUX778076_PARAMS(param, sub) \
    POTHOS_MPL_PP_PARAMS( \
          POTHOS_MPL_PP_SUB(POTHOS_MPL_LIMIT_METAFUNCTION_ARITY, sub) \
        , param \
        ) \
    /**/

#   define AUX778076_SHIFTED_PARAMS(param, sub) \
    POTHOS_MPL_PP_EXT_PARAMS( \
          2, POTHOS_MPL_PP_SUB(POTHOS_PP_INC(POTHOS_MPL_LIMIT_METAFUNCTION_ARITY), sub) \
        , param \
        ) \
    /**/

#   define AUX778076_SPEC_PARAMS(param) \
    POTHOS_MPL_PP_ENUM( \
          POTHOS_PP_DEC(POTHOS_MPL_LIMIT_METAFUNCTION_ARITY) \
        , param \
        ) \
    /**/

namespace aux {

#if !defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

template< bool C_, AUX778076_PARAMS(typename T, 1) >
struct POTHOS_PP_CAT(AUX778076_OP_NAME,impl)
    : POTHOS_PP_CAT(AUX778076_OP_VALUE1,_)
{
};

template< AUX778076_PARAMS(typename T, 1) >
struct POTHOS_PP_CAT(AUX778076_OP_NAME,impl)< AUX778076_OP_VALUE2,AUX778076_PARAMS(T, 1) >
    : POTHOS_PP_CAT(AUX778076_OP_NAME,impl)<
          POTHOS_MPL_AUX_NESTED_TYPE_WKND(T1)::value
        , AUX778076_SHIFTED_PARAMS(T, 1)
        , POTHOS_PP_CAT(AUX778076_OP_VALUE2,_)
        >
{
};

template<>
struct POTHOS_PP_CAT(AUX778076_OP_NAME,impl)<
          AUX778076_OP_VALUE2
        , AUX778076_SPEC_PARAMS(POTHOS_PP_CAT(AUX778076_OP_VALUE2,_))
        >
    : POTHOS_PP_CAT(AUX778076_OP_VALUE2,_)
{
};

#else

template< bool C_ > struct POTHOS_PP_CAT(AUX778076_OP_NAME,impl)
{
    template< AUX778076_PARAMS(typename T, 1) > struct result_
        : POTHOS_PP_CAT(AUX778076_OP_VALUE1,_)
    {
    };
};

template<> struct POTHOS_PP_CAT(AUX778076_OP_NAME,impl)<AUX778076_OP_VALUE2>
{
    template< AUX778076_PARAMS(typename T, 1) > struct result_
        : POTHOS_PP_CAT(AUX778076_OP_NAME,impl)< 
              POTHOS_MPL_AUX_NESTED_TYPE_WKND(T1)::value
            >::template result_< AUX778076_SHIFTED_PARAMS(T,1),POTHOS_PP_CAT(AUX778076_OP_VALUE2,_) >
    {
    };

#if POTHOS_WORKAROUND(POTHOS_MSVC, == 1300)
    template<> struct result_<AUX778076_SPEC_PARAMS(POTHOS_PP_CAT(AUX778076_OP_VALUE2,_))>
        : POTHOS_PP_CAT(AUX778076_OP_VALUE2,_)
    {
    };
};
#else
};

template<>
struct POTHOS_PP_CAT(AUX778076_OP_NAME,impl)<AUX778076_OP_VALUE2>
    ::result_< AUX778076_SPEC_PARAMS(POTHOS_PP_CAT(AUX778076_OP_VALUE2,_)) >
        : POTHOS_PP_CAT(AUX778076_OP_VALUE2,_)
{
};
#endif // BOOST_MSVC == 1300

#endif // BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

} // namespace aux

template<
      typename POTHOS_MPL_AUX_NA_PARAM(T1)
    , typename POTHOS_MPL_AUX_NA_PARAM(T2)
    POTHOS_MPL_PP_DEF_PARAMS_TAIL(2, typename T, POTHOS_PP_CAT(AUX778076_OP_VALUE2,_))
    >
struct AUX778076_OP_NAME
#if !defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
    : aux::POTHOS_PP_CAT(AUX778076_OP_NAME,impl)<
          POTHOS_MPL_AUX_NESTED_TYPE_WKND(T1)::value
        , AUX778076_SHIFTED_PARAMS(T,0)
        >
#else
    : aux::POTHOS_PP_CAT(AUX778076_OP_NAME,impl)< 
          POTHOS_MPL_AUX_NESTED_TYPE_WKND(T1)::value
        >::template result_< AUX778076_SHIFTED_PARAMS(T,0) >
#endif
{
    POTHOS_MPL_AUX_LAMBDA_SUPPORT(
          POTHOS_MPL_LIMIT_METAFUNCTION_ARITY
        , AUX778076_OP_NAME
        , (AUX778076_PARAMS(T, 0))
        )
};

POTHOS_MPL_AUX_NA_SPEC2(
      2
    , POTHOS_MPL_LIMIT_METAFUNCTION_ARITY
    , AUX778076_OP_NAME
    )

}}

#undef AUX778076_SPEC_PARAMS
#undef AUX778076_SHIFTED_PARAMS
#undef AUX778076_PARAMS
#undef AUX778076_OP_NAME
#undef AUX778076_OP_VALUE1
#undef AUX778076_OP_VALUE2
