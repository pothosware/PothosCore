
#ifndef POTHOS_MPL_AUX_ITER_FOLD_IMPL_HPP_INCLUDED
#define POTHOS_MPL_AUX_ITER_FOLD_IMPL_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: iter_fold_impl.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#if !defined(POTHOS_MPL_PREPROCESSING_MODE)
#   include <Pothos/serialization/impl/mpl/next_prior.hpp>
#   include <Pothos/serialization/impl/mpl/apply.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/ctps.hpp>
#   if defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
#       include <Pothos/serialization/impl/mpl/if.hpp>
#       include <Pothos/serialization/impl/type_traits/is_same.hpp>
#   endif
#endif

#include <Pothos/serialization/impl/mpl/aux_/config/use_preprocessed.hpp>

#if !defined(POTHOS_MPL_CFG_NO_PREPROCESSED_HEADERS) \
    && !defined(POTHOS_MPL_PREPROCESSING_MODE)

#   define POTHOS_MPL_PREPROCESSED_HEADER iter_fold_impl.hpp
#   include <Pothos/serialization/impl/mpl/aux_/include_preprocessed.hpp>

#else

#   define AUX778076_FOLD_IMPL_OP(iter) iter
#   define AUX778076_FOLD_IMPL_NAME_PREFIX iter_fold
#   include <Pothos/serialization/impl/mpl/aux_/fold_impl_body.hpp>

#endif // BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#endif // BOOST_MPL_AUX_ITER_FOLD_IMPL_HPP_INCLUDED
