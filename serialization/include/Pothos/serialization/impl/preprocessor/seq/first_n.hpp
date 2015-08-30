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
# ifndef POTHOS_PREPROCESSOR_SEQ_FIRST_N_HPP
# define POTHOS_PREPROCESSOR_SEQ_FIRST_N_HPP
#
# include <Pothos/serialization/impl/preprocessor/config/config.hpp>
# include <Pothos/serialization/impl/preprocessor/control/if.hpp>
# include <Pothos/serialization/impl/preprocessor/seq/detail/split.hpp>
# include <Pothos/serialization/impl/preprocessor/tuple/eat.hpp>
# include <Pothos/serialization/impl/preprocessor/tuple/elem.hpp>
#
# /* POTHOS_PP_SEQ_FIRST_N */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_SEQ_FIRST_N(n, seq) POTHOS_PP_IF(n, POTHOS_PP_TUPLE_ELEM, POTHOS_PP_TUPLE_EAT_3)(2, 0, POTHOS_PP_SEQ_SPLIT(n, seq (nil)))
# else
#    define POTHOS_PP_SEQ_FIRST_N(n, seq) POTHOS_PP_SEQ_FIRST_N_I(n, seq)
#    define POTHOS_PP_SEQ_FIRST_N_I(n, seq) POTHOS_PP_IF(n, POTHOS_PP_TUPLE_ELEM, POTHOS_PP_TUPLE_EAT_3)(2, 0, POTHOS_PP_SEQ_SPLIT(n, seq (nil)))
# endif
#
# endif
