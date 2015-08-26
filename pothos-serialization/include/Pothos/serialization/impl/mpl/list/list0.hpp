
#ifndef POTHOS_MPL_LIST_LIST0_HPP_INCLUDED
#define POTHOS_MPL_LIST_LIST0_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: list0.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/long.hpp>
#include <Pothos/serialization/impl/mpl/aux_/na.hpp>
#include <Pothos/serialization/impl/mpl/list/aux_/push_front.hpp>
#include <Pothos/serialization/impl/mpl/list/aux_/pop_front.hpp>
#include <Pothos/serialization/impl/mpl/list/aux_/push_back.hpp>
#include <Pothos/serialization/impl/mpl/list/aux_/front.hpp>
#include <Pothos/serialization/impl/mpl/list/aux_/clear.hpp>
#include <Pothos/serialization/impl/mpl/list/aux_/O1_size.hpp>
#include <Pothos/serialization/impl/mpl/list/aux_/size.hpp>
#include <Pothos/serialization/impl/mpl/list/aux_/empty.hpp>
#include <Pothos/serialization/impl/mpl/list/aux_/begin_end.hpp>
#include <Pothos/serialization/impl/mpl/list/aux_/item.hpp>

namespace Pothos { namespace mpl {

template< typename Dummy = na > struct list0;

template<> struct list0<na>
    : l_end
{
    typedef l_end type;
};

}}

#endif // BOOST_MPL_LIST_LIST0_HPP_INCLUDED
