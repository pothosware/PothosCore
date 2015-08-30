
#ifndef POTHOS_MPL_AUX_PREPROCESSOR_IS_SEQ_HPP_INCLUDED
#define POTHOS_MPL_AUX_PREPROCESSOR_IS_SEQ_HPP_INCLUDED

// Copyright Paul Mensonides 2003
// Copyright Aleksey Gurtovoy 2003-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: is_seq.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/preprocessor/seq/size.hpp>
#include <Pothos/serialization/impl/preprocessor/arithmetic/dec.hpp>
#include <Pothos/serialization/impl/preprocessor/punctuation/paren.hpp>
#include <Pothos/serialization/impl/preprocessor/cat.hpp>
#include <Pothos/serialization/impl/preprocessor/config/config.hpp>

// returns 1 if 'seq' is a PP-sequence, 0 otherwise:
//
//   BOOST_PP_ASSERT( BOOST_PP_NOT( BOOST_MPL_PP_IS_SEQ( int ) ) )
//   BOOST_PP_ASSERT( BOOST_MPL_PP_IS_SEQ( (int) ) )
//   BOOST_PP_ASSERT( BOOST_MPL_PP_IS_SEQ( (1)(2) ) )

#if (POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_BCC()) || defined(_MSC_VER) && defined(__INTEL_COMPILER) && __INTEL_COMPILER == 1010

#   define POTHOS_MPL_PP_IS_SEQ(seq) POTHOS_PP_DEC( POTHOS_PP_SEQ_SIZE( POTHOS_MPL_PP_IS_SEQ_(seq) ) )
#   define POTHOS_MPL_PP_IS_SEQ_(seq) POTHOS_MPL_PP_IS_SEQ_SEQ_( POTHOS_MPL_PP_IS_SEQ_SPLIT_ seq )
#   define POTHOS_MPL_PP_IS_SEQ_SEQ_(x) (x)
#   define POTHOS_MPL_PP_IS_SEQ_SPLIT_(unused) unused)((unused)

#else

#   if POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_MWCC()
#       define POTHOS_MPL_PP_IS_SEQ(seq) POTHOS_MPL_PP_IS_SEQ_MWCC_((seq))
#       define POTHOS_MPL_PP_IS_SEQ_MWCC_(args) POTHOS_MPL_PP_IS_SEQ_ ## args
#   else
#       define POTHOS_MPL_PP_IS_SEQ(seq) POTHOS_MPL_PP_IS_SEQ_(seq)
#   endif

#   define POTHOS_MPL_PP_IS_SEQ_(seq) POTHOS_PP_CAT(POTHOS_MPL_PP_IS_SEQ_, POTHOS_MPL_PP_IS_SEQ_0 seq POTHOS_PP_RPAREN())
#   define POTHOS_MPL_PP_IS_SEQ_0(x) POTHOS_MPL_PP_IS_SEQ_1(x
#   define POTHOS_MPL_PP_IS_SEQ_ALWAYS_0(unused) 0
#   define POTHOS_MPL_PP_IS_SEQ_POTHOS_MPL_PP_IS_SEQ_0 POTHOS_MPL_PP_IS_SEQ_ALWAYS_0(
#   define POTHOS_MPL_PP_IS_SEQ_POTHOS_MPL_PP_IS_SEQ_1(unused) 1

#endif

#endif // BOOST_MPL_AUX_PREPROCESSOR_IS_SEQ_HPP_INCLUDED
