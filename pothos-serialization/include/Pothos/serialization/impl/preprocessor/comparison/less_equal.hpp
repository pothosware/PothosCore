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
# ifndef POTHOS_PREPROCESSOR_COMPARISON_LESS_EQUAL_HPP
# define POTHOS_PREPROCESSOR_COMPARISON_LESS_EQUAL_HPP
#
# include <Pothos/serialization/impl/preprocessor/arithmetic/sub.hpp>
# include <Pothos/serialization/impl/preprocessor/config/config.hpp>
# include <Pothos/serialization/impl/preprocessor/logical/not.hpp>
#
# /* POTHOS_PP_LESS_EQUAL */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_LESS_EQUAL(x, y) POTHOS_PP_NOT(POTHOS_PP_SUB(x, y))
# else
#    define POTHOS_PP_LESS_EQUAL(x, y) POTHOS_PP_LESS_EQUAL_I(x, y)
#    define POTHOS_PP_LESS_EQUAL_I(x, y) POTHOS_PP_NOT(POTHOS_PP_SUB(x, y))
# endif
#
# /* POTHOS_PP_LESS_EQUAL_D */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_LESS_EQUAL_D(d, x, y) POTHOS_PP_NOT(POTHOS_PP_SUB_D(d, x, y))
# else
#    define POTHOS_PP_LESS_EQUAL_D(d, x, y) POTHOS_PP_LESS_EQUAL_D_I(d, x, y)
#    define POTHOS_PP_LESS_EQUAL_D_I(d, x, y) POTHOS_PP_NOT(POTHOS_PP_SUB_D(d, x, y))
# endif
#
# endif
