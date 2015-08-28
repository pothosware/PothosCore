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
# ifndef POTHOS_PREPROCESSOR_LIST_FOLD_RIGHT_HPP
# define POTHOS_PREPROCESSOR_LIST_FOLD_RIGHT_HPP
#
# include <Pothos/serialization/impl/preprocessor/cat.hpp>
# include <Pothos/serialization/impl/preprocessor/control/while.hpp>
# include <Pothos/serialization/impl/preprocessor/debug/error.hpp>
# include <Pothos/serialization/impl/preprocessor/detail/auto_rec.hpp>
#
# if 0
#    define POTHOS_PP_LIST_FOLD_RIGHT(op, state, list)
# endif
#
# define POTHOS_PP_LIST_FOLD_RIGHT POTHOS_PP_CAT(POTHOS_PP_LIST_FOLD_RIGHT_, POTHOS_PP_AUTO_REC(POTHOS_PP_WHILE_P, 256))
#
# define POTHOS_PP_LIST_FOLD_RIGHT_257(o, s, l) POTHOS_PP_ERROR(0x0004)
#
# define POTHOS_PP_LIST_FOLD_RIGHT_D(d, o, s, l) POTHOS_PP_LIST_FOLD_RIGHT_ ## d(o, s, l)
# define POTHOS_PP_LIST_FOLD_RIGHT_2ND POTHOS_PP_LIST_FOLD_RIGHT
# define POTHOS_PP_LIST_FOLD_RIGHT_2ND_D POTHOS_PP_LIST_FOLD_RIGHT_D
#
# if POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    include <Pothos/serialization/impl/preprocessor/list/detail/edg/fold_right.hpp>
# else
#    include <Pothos/serialization/impl/preprocessor/list/detail/fold_right.hpp>
# endif
#
# endif
