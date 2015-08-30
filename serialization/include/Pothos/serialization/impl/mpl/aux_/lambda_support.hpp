
#ifndef POTHOS_MPL_AUX_LAMBDA_SUPPORT_HPP_INCLUDED
#define POTHOS_MPL_AUX_LAMBDA_SUPPORT_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2001-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: lambda_support.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/aux_/config/lambda.hpp>

#if !defined(POTHOS_MPL_CFG_NO_FULL_LAMBDA_SUPPORT)

#   define POTHOS_MPL_AUX_LAMBDA_SUPPORT_SPEC(i, name, params) /**/
#   define POTHOS_MPL_AUX_LAMBDA_SUPPORT(i,name,params) /**/

#else

#   include <Pothos/serialization/impl/mpl/int_fwd.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/yes_no.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/na_fwd.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/params.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/enum.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/msvc.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/workaround.hpp>

#   include <Pothos/serialization/impl/preprocessor/tuple/to_list.hpp>
#   include <Pothos/serialization/impl/preprocessor/list/for_each_i.hpp>
#   include <Pothos/serialization/impl/preprocessor/inc.hpp>
#   include <Pothos/serialization/impl/preprocessor/cat.hpp>

#   define POTHOS_MPL_AUX_LAMBDA_SUPPORT_ARG_TYPEDEF_FUNC(R,typedef_,i,param) \
    typedef_ param POTHOS_PP_CAT(arg,POTHOS_PP_INC(i)); \
    /**/

// agurt, 07/mar/03: restore an old revision for the sake of SGI MIPSpro C++
#if POTHOS_WORKAROUND(__EDG_VERSION__, <= 238) 

#   define POTHOS_MPL_AUX_LAMBDA_SUPPORT(i, name, params) \
    typedef POTHOS_MPL_AUX_ADL_BARRIER_NAMESPACE::int_<i> arity; \
    POTHOS_PP_LIST_FOR_EACH_I_R( \
          1 \
        , POTHOS_MPL_AUX_LAMBDA_SUPPORT_ARG_TYPEDEF_FUNC \
        , typedef \
        , POTHOS_PP_TUPLE_TO_LIST(i,params) \
        ) \
    struct rebind \
    { \
        template< POTHOS_MPL_PP_PARAMS(i,typename U) > struct apply \
            : name< POTHOS_MPL_PP_PARAMS(i,U) > \
        { \
        }; \
    }; \
    /**/

#   define POTHOS_MPL_AUX_LAMBDA_SUPPORT_SPEC(i, name, params) \
    POTHOS_MPL_AUX_LAMBDA_SUPPORT(i, name, params) \
    /**/

#elif POTHOS_WORKAROUND(__EDG_VERSION__, <= 244) && !defined(POTHOS_INTEL_CXX_VERSION)
// agurt, 18/jan/03: old EDG-based compilers actually enforce 11.4 para 9
// (in strict mode), so we have to provide an alternative to the 
// MSVC-optimized implementation

#   define POTHOS_MPL_AUX_LAMBDA_SUPPORT_SPEC(i, name, params) \
    typedef POTHOS_MPL_AUX_ADL_BARRIER_NAMESPACE::int_<i> arity; \
    POTHOS_PP_LIST_FOR_EACH_I_R( \
          1 \
        , POTHOS_MPL_AUX_LAMBDA_SUPPORT_ARG_TYPEDEF_FUNC \
        , typedef \
        , POTHOS_PP_TUPLE_TO_LIST(i,params) \
        ) \
    struct rebind; \
/**/

#   define POTHOS_MPL_AUX_LAMBDA_SUPPORT(i, name, params) \
    POTHOS_MPL_AUX_LAMBDA_SUPPORT_SPEC(i, name, params) \
}; \
template< POTHOS_MPL_PP_PARAMS(i,typename T) > \
struct name<POTHOS_MPL_PP_PARAMS(i,T)>::rebind \
{ \
    template< POTHOS_MPL_PP_PARAMS(i,typename U) > struct apply \
        : name< POTHOS_MPL_PP_PARAMS(i,U) > \
    { \
    }; \
/**/

#else // __EDG_VERSION__

namespace Pothos { namespace mpl { namespace aux {
template< typename T > struct has_rebind_tag;
}}}

#   define POTHOS_MPL_AUX_LAMBDA_SUPPORT_SPEC(i, name, params) \
    typedef POTHOS_MPL_AUX_ADL_BARRIER_NAMESPACE::int_<i> arity; \
    POTHOS_PP_LIST_FOR_EACH_I_R( \
          1 \
        , POTHOS_MPL_AUX_LAMBDA_SUPPORT_ARG_TYPEDEF_FUNC \
        , typedef \
        , POTHOS_PP_TUPLE_TO_LIST(i,params) \
        ) \
    friend class POTHOS_PP_CAT(name,_rebind); \
    typedef POTHOS_PP_CAT(name,_rebind) rebind; \
/**/

#if POTHOS_WORKAROUND(__BORLANDC__, POTHOS_TESTED_AT(0x610))
#   define POTHOS_MPL_AUX_LAMBDA_SUPPORT_HAS_REBIND(i, name, params) \
template< POTHOS_MPL_PP_PARAMS(i,typename T) > \
::Pothos::mpl::aux::yes_tag operator|( \
      ::Pothos::mpl::aux::has_rebind_tag<int> \
    , name<POTHOS_MPL_PP_PARAMS(i,T)>* \
    ); \
::Pothos::mpl::aux::no_tag operator|( \
      ::Pothos::mpl::aux::has_rebind_tag<int> \
    , name< POTHOS_MPL_PP_ENUM(i,::Pothos::mpl::na) >* \
    ); \
/**/
#elif !POTHOS_WORKAROUND(POTHOS_MSVC, < 1300)
#   define POTHOS_MPL_AUX_LAMBDA_SUPPORT_HAS_REBIND(i, name, params) \
template< POTHOS_MPL_PP_PARAMS(i,typename T) > \
::Pothos::mpl::aux::yes_tag operator|( \
      ::Pothos::mpl::aux::has_rebind_tag<int> \
    , ::Pothos::mpl::aux::has_rebind_tag< name<POTHOS_MPL_PP_PARAMS(i,T)> >* \
    ); \
/**/
#else
#   define POTHOS_MPL_AUX_LAMBDA_SUPPORT_HAS_REBIND(i, name, params) /**/
#endif

#   if !defined(__BORLANDC__)
#   define POTHOS_MPL_AUX_LAMBDA_SUPPORT(i, name, params) \
    POTHOS_MPL_AUX_LAMBDA_SUPPORT_SPEC(i, name, params) \
}; \
POTHOS_MPL_AUX_LAMBDA_SUPPORT_HAS_REBIND(i, name, params) \
class POTHOS_PP_CAT(name,_rebind) \
{ \
 public: \
    template< POTHOS_MPL_PP_PARAMS(i,typename U) > struct apply \
        : name< POTHOS_MPL_PP_PARAMS(i,U) > \
    { \
    }; \
/**/
#   else
#   define POTHOS_MPL_AUX_LAMBDA_SUPPORT(i, name, params) \
    POTHOS_MPL_AUX_LAMBDA_SUPPORT_SPEC(i, name, params) \
}; \
POTHOS_MPL_AUX_LAMBDA_SUPPORT_HAS_REBIND(i, name, params) \
class POTHOS_PP_CAT(name,_rebind) \
{ \
 public: \
    template< POTHOS_MPL_PP_PARAMS(i,typename U) > struct apply \
    { \
        typedef typename name< POTHOS_MPL_PP_PARAMS(i,U) >::type type; \
    }; \
/**/
#   endif // __BORLANDC__

#endif // __EDG_VERSION__

#endif // BOOST_MPL_CFG_NO_FULL_LAMBDA_SUPPORT

#endif // BOOST_MPL_AUX_LAMBDA_SUPPORT_HPP_INCLUDED
