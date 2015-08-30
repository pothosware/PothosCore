
#ifndef POTHOS_MPL_DEREF_HPP_INCLUDED
#define POTHOS_MPL_DEREF_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2002-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: deref.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/aux_/msvc_type.hpp>
#include <Pothos/serialization/impl/mpl/aux_/na_spec.hpp>
#include <Pothos/serialization/impl/mpl/aux_/lambda_support.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/eti.hpp>

namespace Pothos { namespace mpl {

template<
      typename POTHOS_MPL_AUX_NA_PARAM(Iterator)
    >
struct deref
{
#if !defined(POTHOS_MPL_CFG_MSVC_70_ETI_BUG)
    typedef typename Iterator::type type;
#else
    typedef typename aux::msvc_type<Iterator>::type type;
#endif
    POTHOS_MPL_AUX_LAMBDA_SUPPORT(1,deref,(Iterator))
};

POTHOS_MPL_AUX_NA_SPEC(1, deref)

}}

#endif // BOOST_MPL_DEREF_HPP_INCLUDED
