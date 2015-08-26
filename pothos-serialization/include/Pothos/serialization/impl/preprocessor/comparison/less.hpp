# /* Copyright (C) 2001
#  * Housemarque Oy
#  * http://www.housemarque.com
#  *
#  * Distributed under the Boost Software License, Version 1.0. (See
#  * accompanying file LICENSE_1_0.txt or copy at
#  * http://www.boost.org/LICENSE_1_0.txt)
#  */
#
# /* Revised by Paul Mensonides (2002) */
#
# /* See http://www.boost.org for most recent version. */
#
# ifndef POTHOS_PREPROCESSOR_COMPARISON_LESS_HPP
# define POTHOS_PREPROCESSOR_COMPARISON_LESS_HPP
#
# include <Pothos/serialization/impl/preprocessor/comparison/less_equal.hpp>
# include <Pothos/serialization/impl/preprocessor/comparison/not_equal.hpp>
# include <Pothos/serialization/impl/preprocessor/config/config.hpp>
# include <Pothos/serialization/impl/preprocessor/control/iif.hpp>
# include <Pothos/serialization/impl/preprocessor/logical/bitand.hpp>
# include <Pothos/serialization/impl/preprocessor/tuple/eat.hpp>
#
# /* POTHOS_PP_LESS */
#
# if POTHOS_PP_CONFIG_FLAGS() & (POTHOS_PP_CONFIG_MWCC() | POTHOS_PP_CONFIG_DMC())
#    define POTHOS_PP_LESS(x, y) POTHOS_PP_BITAND(POTHOS_PP_NOT_EQUAL(x, y), POTHOS_PP_LESS_EQUAL(x, y))
# elif ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_LESS(x, y) POTHOS_PP_IIF(POTHOS_PP_NOT_EQUAL(x, y), POTHOS_PP_LESS_EQUAL, 0 POTHOS_PP_TUPLE_EAT_2)(x, y)
# else
#    define POTHOS_PP_LESS(x, y) POTHOS_PP_LESS_I(x, y)
#    define POTHOS_PP_LESS_I(x, y) POTHOS_PP_IIF(POTHOS_PP_NOT_EQUAL(x, y), POTHOS_PP_LESS_EQUAL, 0 POTHOS_PP_TUPLE_EAT_2)(x, y)
# endif
#
# /* POTHOS_PP_LESS_D */
#
# if POTHOS_PP_CONFIG_FLAGS() & (POTHOS_PP_CONFIG_MWCC() | POTHOS_PP_CONFIG_DMC())
#    define POTHOS_PP_LESS_D(d, x, y) POTHOS_PP_BITAND(POTHOS_PP_NOT_EQUAL(x, y), POTHOS_PP_LESS_EQUAL_D(d, x, y))
# elif ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_LESS_D(d, x, y) POTHOS_PP_IIF(POTHOS_PP_NOT_EQUAL(x, y), POTHOS_PP_LESS_EQUAL_D, 0 POTHOS_PP_TUPLE_EAT_3)(d, x, y)
# else
#    define POTHOS_PP_LESS_D(d, x, y) POTHOS_PP_LESS_D_I(d, x, y)
#    define POTHOS_PP_LESS_D_I(d, x, y) POTHOS_PP_IIF(POTHOS_PP_NOT_EQUAL(x, y), POTHOS_PP_LESS_EQUAL_D, 0 POTHOS_PP_TUPLE_EAT_3)(d, x, y)
# endif
#
# endif
