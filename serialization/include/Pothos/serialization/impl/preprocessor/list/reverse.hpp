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
# ifndef POTHOS_PREPROCESSOR_LIST_REVERSE_HPP
# define POTHOS_PREPROCESSOR_LIST_REVERSE_HPP
#
# include <Pothos/serialization/impl/preprocessor/config/config.hpp>
# include <Pothos/serialization/impl/preprocessor/list/fold_left.hpp>
#
# /* POTHOS_PP_LIST_REVERSE */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_LIST_REVERSE(list) POTHOS_PP_LIST_FOLD_LEFT(POTHOS_PP_LIST_REVERSE_O, POTHOS_PP_NIL, list)
# else
#    define POTHOS_PP_LIST_REVERSE(list) POTHOS_PP_LIST_REVERSE_I(list)
#    define POTHOS_PP_LIST_REVERSE_I(list) POTHOS_PP_LIST_FOLD_LEFT(POTHOS_PP_LIST_REVERSE_O, POTHOS_PP_NIL, list)
# endif
#
# define POTHOS_PP_LIST_REVERSE_O(d, s, x) (x, s)
#
# /* POTHOS_PP_LIST_REVERSE_D */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_LIST_REVERSE_D(d, list) POTHOS_PP_LIST_FOLD_LEFT_ ## d(POTHOS_PP_LIST_REVERSE_O, POTHOS_PP_NIL, list)
# else
#    define POTHOS_PP_LIST_REVERSE_D(d, list) POTHOS_PP_LIST_REVERSE_D_I(d, list)
#    define POTHOS_PP_LIST_REVERSE_D_I(d, list) POTHOS_PP_LIST_FOLD_LEFT_ ## d(POTHOS_PP_LIST_REVERSE_O, POTHOS_PP_NIL, list)
# endif
#
# endif
