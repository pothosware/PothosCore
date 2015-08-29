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
# ifndef POTHOS_PREPROCESSOR_COMPARISON_GREATER_HPP
# define POTHOS_PREPROCESSOR_COMPARISON_GREATER_HPP
#
# include <Pothos/serialization/impl/preprocessor/comparison/less.hpp>
# include <Pothos/serialization/impl/preprocessor/config/config.hpp>
#
# /* POTHOS_PP_GREATER */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_GREATER(x, y) POTHOS_PP_LESS(y, x)
# else
#    define POTHOS_PP_GREATER(x, y) POTHOS_PP_GREATER_I(x, y)
#    define POTHOS_PP_GREATER_I(x, y) POTHOS_PP_LESS(y, x)
# endif
#
# /* POTHOS_PP_GREATER_D */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_GREATER_D(d, x, y) POTHOS_PP_LESS_D(d, y, x)
# else
#    define POTHOS_PP_GREATER_D(d, x, y) POTHOS_PP_GREATER_D_I(d, x, y)
#    define POTHOS_PP_GREATER_D_I(d, x, y) POTHOS_PP_LESS_D(d, y, x)
# endif
#
# endif
