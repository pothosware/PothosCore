
#ifndef POTHOS_MPL_AUX_RANGE_C_SIZE_HPP_INCLUDED
#define POTHOS_MPL_AUX_RANGE_C_SIZE_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: size.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/size_fwd.hpp>
#include <Pothos/serialization/impl/mpl/minus.hpp>
#include <Pothos/serialization/impl/mpl/aux_/range_c/tag.hpp>

namespace Pothos { namespace mpl {

template<>
struct size_impl< aux::half_open_range_tag >
{
    template< typename Range > struct apply
        : minus<
              typename Range::finish
            , typename Range::start
            >
    {
    };
};

}}

#endif // BOOST_MPL_AUX_RANGE_C_SIZE_HPP_INCLUDED
