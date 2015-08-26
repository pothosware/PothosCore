
#ifndef POTHOS_MPL_VECTOR_AUX_BACK_HPP_INCLUDED
#define POTHOS_MPL_VECTOR_AUX_BACK_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: back.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/back_fwd.hpp>
#include <Pothos/serialization/impl/mpl/next_prior.hpp>
#include <Pothos/serialization/impl/mpl/vector/aux_/at.hpp>
#include <Pothos/serialization/impl/mpl/vector/aux_/tag.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/typeof.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/ctps.hpp>

namespace Pothos { namespace mpl {

#if defined(POTHOS_MPL_CFG_TYPEOF_BASED_SEQUENCES)

template<>
struct back_impl< aux::vector_tag >
{
    template< typename Vector > struct apply
        : v_at<
              Vector
            , prior<typename Vector::size>::type::value
            >
    {
    };
};

#else

#if !defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

template< long n_ >
struct back_impl< aux::vector_tag<n_> >
{
    template< typename Vector > struct apply
    {
        typedef typename Vector::back type;
    };
};

#endif // BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

#endif // BOOST_MPL_CFG_TYPEOF_BASED_SEQUENCES

}}

#endif // BOOST_MPL_VECTOR_AUX_BACK_HPP_INCLUDED
