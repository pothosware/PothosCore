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
# ifndef POTHOS_PREPROCESSOR_LIST_APPEND_HPP
# define POTHOS_PREPROCESSOR_LIST_APPEND_HPP
#
# include <Pothos/serialization/impl/preprocessor/config/config.hpp>
# include <Pothos/serialization/impl/preprocessor/list/fold_right.hpp>
#
# /* POTHOS_PP_LIST_APPEND */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_LIST_APPEND(a, b) POTHOS_PP_LIST_FOLD_RIGHT(POTHOS_PP_LIST_APPEND_O, b, a)
# else
#    define POTHOS_PP_LIST_APPEND(a, b) POTHOS_PP_LIST_APPEND_I(a, b)
#    define POTHOS_PP_LIST_APPEND_I(a, b) POTHOS_PP_LIST_FOLD_RIGHT(POTHOS_PP_LIST_APPEND_O, b, a)
# endif
#
# define POTHOS_PP_LIST_APPEND_O(d, s, x) (x, s)
#
# /* POTHOS_PP_LIST_APPEND_D */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_LIST_APPEND_D(d, a, b) POTHOS_PP_LIST_FOLD_RIGHT_ ## d(POTHOS_PP_LIST_APPEND_O, b, a)
# else
#    define POTHOS_PP_LIST_APPEND_D(d, a, b) POTHOS_PP_LIST_APPEND_D_I(d, a, b)
#    define POTHOS_PP_LIST_APPEND_D_I(d, a, b) POTHOS_PP_LIST_FOLD_RIGHT_ ## d(POTHOS_PP_LIST_APPEND_O, b, a)
# endif
#
# endif
