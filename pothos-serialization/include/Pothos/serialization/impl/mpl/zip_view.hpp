
#ifndef POTHOS_MPL_ZIP_VIEW_HPP_INCLUDED
#define POTHOS_MPL_ZIP_VIEW_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2010
// Copyright David Abrahams 2000-2002
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: zip_view.hpp 61591 2010-04-26 21:31:09Z agurtovoy $
// $Date: 2010-04-26 14:31:09 -0700 (Mon, 26 Apr 2010) $
// $Revision: 61591 $

#include <Pothos/serialization/impl/mpl/transform.hpp>
#include <Pothos/serialization/impl/mpl/begin_end.hpp>
#include <Pothos/serialization/impl/mpl/iterator_tags.hpp>
#include <Pothos/serialization/impl/mpl/next.hpp>
#include <Pothos/serialization/impl/mpl/lambda.hpp>
#include <Pothos/serialization/impl/mpl/deref.hpp>
#include <Pothos/serialization/impl/mpl/aux_/na_spec.hpp>

namespace Pothos { namespace mpl {

template< typename IteratorSeq >
struct zip_iterator
{
    typedef forward_iterator_tag category;
    typedef typename transform1<
          IteratorSeq
        , deref<_1>
        >::type type;

    typedef zip_iterator<
          typename transform1<
                IteratorSeq
              , mpl::next<_1>
            >::type
        > next;
};

template<
      typename POTHOS_MPL_AUX_NA_PARAM(Sequences)
    >
struct zip_view
{
 private:
    typedef typename transform1< Sequences, mpl::begin<_1> >::type first_ones_;
    typedef typename transform1< Sequences, mpl::end<_1> >::type last_ones_;
    
 public:
    typedef nested_begin_end_tag tag;
    typedef zip_view type;
    typedef zip_iterator<first_ones_> begin;
    typedef zip_iterator<last_ones_> end;
};

POTHOS_MPL_AUX_NA_SPEC(1, zip_view)

}}

#endif // BOOST_MPL_ZIP_VIEW_HPP_INCLUDED
