// Copyright (C) 2003, Fernando Luis Cacciola Carballal.
// Copyright (C) 2007, Tobias Schwinger.
//
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/optional for documentation.
//
// You are welcome to contact the author at:
//  fernando_cacciola@hotmail.com
//
#ifndef POTHOS_UTILITY_DETAIL_INPLACE_FACTORY_PREFIX_04APR2007_HPP
#define POTHOS_UTILITY_DETAIL_INPLACE_FACTORY_PREFIX_04APR2007_HPP

#include <new>
#include <cstddef>
#include <Pothos/serialization/impl/config.hpp>
#include <Pothos/serialization/impl/preprocessor/cat.hpp>
#include <Pothos/serialization/impl/preprocessor/punctuation/paren.hpp>
#include <Pothos/serialization/impl/preprocessor/iteration/iterate.hpp>
#include <Pothos/serialization/impl/preprocessor/repetition/repeat.hpp>
#include <Pothos/serialization/impl/preprocessor/repetition/enum.hpp>
#include <Pothos/serialization/impl/preprocessor/repetition/enum_params.hpp>
#include <Pothos/serialization/impl/preprocessor/repetition/enum_binary_params.hpp>
#include <Pothos/serialization/impl/preprocessor/repetition/enum_trailing_params.hpp>

#define POTHOS_DEFINE_INPLACE_FACTORY_CLASS_MEMBER_INIT(z,n,_) POTHOS_PP_CAT(m_a,n) POTHOS_PP_LPAREN() POTHOS_PP_CAT(a,n) POTHOS_PP_RPAREN()
#define POTHOS_DEFINE_INPLACE_FACTORY_CLASS_MEMBER_DECL(z,n,_) POTHOS_PP_CAT(A,n) const& POTHOS_PP_CAT(m_a,n);

#define POTHOS_MAX_INPLACE_FACTORY_ARITY 10

#undef POTHOS_UTILITY_DETAIL_INPLACE_FACTORY_SUFFIX_04APR2007_HPP

#endif

