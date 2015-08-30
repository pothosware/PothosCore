
#ifndef POTHOS_MPL_AUX_ADL_BARRIER_HPP_INCLUDED
#define POTHOS_MPL_AUX_ADL_BARRIER_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2002-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: adl_barrier.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/aux_/config/adl.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/gcc.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/workaround.hpp>

#if !defined(POTHOS_MPL_CFG_NO_ADL_BARRIER_NAMESPACE)

#   define POTHOS_MPL_AUX_ADL_BARRIER_NAMESPACE mpl_
#   define POTHOS_MPL_AUX_ADL_BARRIER_NAMESPACE_OPEN namespace mpl_ {
#   define POTHOS_MPL_AUX_ADL_BARRIER_NAMESPACE_CLOSE }
#   define POTHOS_MPL_AUX_ADL_BARRIER_DECL(type) \
    namespace Pothos { namespace mpl { \
    using ::POTHOS_MPL_AUX_ADL_BARRIER_NAMESPACE::type; \
    } } \
/**/

#if !defined(POTHOS_MPL_PREPROCESSING_MODE)
namespace POTHOS_MPL_AUX_ADL_BARRIER_NAMESPACE { namespace aux {} }
namespace Pothos { namespace mpl { using namespace POTHOS_MPL_AUX_ADL_BARRIER_NAMESPACE; 
namespace aux { using namespace POTHOS_MPL_AUX_ADL_BARRIER_NAMESPACE::aux; }
}}
#endif

#else // BOOST_MPL_CFG_NO_ADL_BARRIER_NAMESPACE

#   define POTHOS_MPL_AUX_ADL_BARRIER_NAMESPACE Pothos::mpl
#   define POTHOS_MPL_AUX_ADL_BARRIER_NAMESPACE_OPEN namespace Pothos { namespace mpl {
#   define POTHOS_MPL_AUX_ADL_BARRIER_NAMESPACE_CLOSE }}
#   define POTHOS_MPL_AUX_ADL_BARRIER_DECL(type) /**/

#endif

#endif // BOOST_MPL_AUX_ADL_BARRIER_HPP_INCLUDED
