# /* **************************************************************************
#  *                                                                          *
#  *     (C) Copyright Paul Mensonides 2002.
#  *     Distributed under the Boost Software License, Version 1.0. (See
#  *     accompanying file LICENSE_1_0.txt or copy at
#  *     http://www.boost.org/LICENSE_1_0.txt)
#  *                                                                          *
#  ************************************************************************** */
#
# /* See http://www.boost.org for most recent version. */
#
# ifndef POTHOS_PREPROCESSOR_SEQ_REST_N_HPP
# define POTHOS_PREPROCESSOR_SEQ_REST_N_HPP
#
# include <Pothos/serialization/impl/preprocessor/arithmetic/inc.hpp>
# include <Pothos/serialization/impl/preprocessor/config/config.hpp>
# include <Pothos/serialization/impl/preprocessor/facilities/empty.hpp>
# include <Pothos/serialization/impl/preprocessor/seq/detail/split.hpp>
# include <Pothos/serialization/impl/preprocessor/tuple/elem.hpp>
#
# /* POTHOS_PP_SEQ_REST_N */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_SEQ_REST_N(n, seq) POTHOS_PP_TUPLE_ELEM(2, 1, POTHOS_PP_SEQ_SPLIT(POTHOS_PP_INC(n), (nil) seq POTHOS_PP_EMPTY))()
# else
#    define POTHOS_PP_SEQ_REST_N(n, seq) POTHOS_PP_SEQ_REST_N_I(n, seq)
#    define POTHOS_PP_SEQ_REST_N_I(n, seq) POTHOS_PP_TUPLE_ELEM(2, 1, POTHOS_PP_SEQ_SPLIT(POTHOS_PP_INC(n), (nil) seq POTHOS_PP_EMPTY))()
# endif
#
# endif
