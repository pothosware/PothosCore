
#ifndef POTHOS_MPL_SEQUENCE_TAG_HPP_INCLUDED
#define POTHOS_MPL_SEQUENCE_TAG_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: sequence_tag.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/mpl/sequence_tag_fwd.hpp>
#include <Pothos/serialization/impl/mpl/aux_/has_tag.hpp>
#include <Pothos/serialization/impl/mpl/aux_/has_begin.hpp>
#include <Pothos/serialization/impl/mpl/aux_/na_spec.hpp>
#include <Pothos/serialization/impl/mpl/aux_/is_msvc_eti_arg.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/eti.hpp>
#include <Pothos/serialization/impl/mpl/aux_/yes_no.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/workaround.hpp>

namespace Pothos { namespace mpl {

// agurt, 27/nov/02: have to use a simplistic 'sequence_tag' implementation
// on MSVC to avoid dreadful "internal structure overflow" error
#if POTHOS_WORKAROUND(POTHOS_MSVC, < 1300) \
    || defined(POTHOS_MPL_CFG_NO_HAS_XXX)

template<
      typename POTHOS_MPL_AUX_NA_PARAM(Sequence)
    >
struct sequence_tag
{
    typedef typename Sequence::tag type;
};

#elif POTHOS_WORKAROUND(POTHOS_MSVC, == 1300)

// agurt, 07/feb/03: workaround for what seems to be MSVC 7.0-specific ETI issue

namespace aux {

template< bool >
struct sequence_tag_impl
{
    template< typename Sequence > struct result_
    {
        typedef typename Sequence::tag type;
    };
};

template<>
struct sequence_tag_impl<false>
{
    template< typename Sequence > struct result_
    {
        typedef int type;
    };
};

} // namespace aux

template<
      typename POTHOS_MPL_AUX_NA_PARAM(Sequence)
    >
struct sequence_tag
    : aux::sequence_tag_impl< !aux::is_msvc_eti_arg<Sequence>::value >
        ::template result_<Sequence>
{
};

#else

namespace aux {

template< bool has_tag_, bool has_begin_ >
struct sequence_tag_impl
{
    // agurt 24/nov/02: MSVC 6.5 gets confused in 'sequence_tag_impl<true>' 
    // specialization below, if we name it 'result_' here
    template< typename Sequence > struct result2_;
};

#   define AUX_CLASS_SEQUENCE_TAG_SPEC(has_tag, has_begin, result_type) \
template<> struct sequence_tag_impl<has_tag,has_begin> \
{ \
    template< typename Sequence > struct result2_ \
    { \
        typedef result_type type; \
    }; \
}; \
/**/

AUX_CLASS_SEQUENCE_TAG_SPEC(true, true, typename Sequence::tag)
AUX_CLASS_SEQUENCE_TAG_SPEC(true, false, typename Sequence::tag)
AUX_CLASS_SEQUENCE_TAG_SPEC(false, true, nested_begin_end_tag)
AUX_CLASS_SEQUENCE_TAG_SPEC(false, false, non_sequence_tag)

#   undef AUX_CLASS_SEQUENCE_TAG_SPEC

} // namespace aux

template<
      typename POTHOS_MPL_AUX_NA_PARAM(Sequence)
    >
struct sequence_tag
    : aux::sequence_tag_impl<
          ::Pothos::mpl::aux::has_tag<Sequence>::value
        , ::Pothos::mpl::aux::has_begin<Sequence>::value
        >::template result2_<Sequence>
{
};

#endif // BOOST_MSVC

POTHOS_MPL_AUX_NA_SPEC(1, sequence_tag)

}}

#endif // BOOST_MPL_SEQUENCE_TAG_HPP_INCLUDED
