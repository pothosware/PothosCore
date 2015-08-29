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
# ifndef POTHOS_PREPROCESSOR_REPETITION_REPEAT_FROM_TO_HPP
# define POTHOS_PREPROCESSOR_REPETITION_REPEAT_FROM_TO_HPP
#
# include <Pothos/serialization/impl/preprocessor/arithmetic/add.hpp>
# include <Pothos/serialization/impl/preprocessor/arithmetic/sub.hpp>
# include <Pothos/serialization/impl/preprocessor/cat.hpp>
# include <Pothos/serialization/impl/preprocessor/config/config.hpp>
# include <Pothos/serialization/impl/preprocessor/control/while.hpp>
# include <Pothos/serialization/impl/preprocessor/debug/error.hpp>
# include <Pothos/serialization/impl/preprocessor/detail/auto_rec.hpp>
# include <Pothos/serialization/impl/preprocessor/repetition/repeat.hpp>
# include <Pothos/serialization/impl/preprocessor/tuple/elem.hpp>
# include <Pothos/serialization/impl/preprocessor/tuple/rem.hpp>
#
# /* POTHOS_PP_REPEAT_FROM_TO */
#
# if 0
#    define POTHOS_PP_REPEAT_FROM_TO(first, last, macro, data)
# endif
#
# define POTHOS_PP_REPEAT_FROM_TO POTHOS_PP_CAT(POTHOS_PP_REPEAT_FROM_TO_, POTHOS_PP_AUTO_REC(POTHOS_PP_REPEAT_P, 4))
#
# define POTHOS_PP_REPEAT_FROM_TO_1(f, l, m, dt) POTHOS_PP_REPEAT_FROM_TO_D_1(POTHOS_PP_AUTO_REC(POTHOS_PP_WHILE_P, 256), f, l, m, dt)
# define POTHOS_PP_REPEAT_FROM_TO_2(f, l, m, dt) POTHOS_PP_REPEAT_FROM_TO_D_2(POTHOS_PP_AUTO_REC(POTHOS_PP_WHILE_P, 256), f, l, m, dt)
# define POTHOS_PP_REPEAT_FROM_TO_3(f, l, m, dt) POTHOS_PP_REPEAT_FROM_TO_D_3(POTHOS_PP_AUTO_REC(POTHOS_PP_WHILE_P, 256), f, l, m, dt)
# define POTHOS_PP_REPEAT_FROM_TO_4(f, l, m, dt) POTHOS_PP_ERROR(0x0003)
#
# define POTHOS_PP_REPEAT_FROM_TO_1ST POTHOS_PP_REPEAT_FROM_TO_1
# define POTHOS_PP_REPEAT_FROM_TO_2ND POTHOS_PP_REPEAT_FROM_TO_2
# define POTHOS_PP_REPEAT_FROM_TO_3RD POTHOS_PP_REPEAT_FROM_TO_3
#
# /* POTHOS_PP_REPEAT_FROM_TO_D */
#
# if 0
#    define POTHOS_PP_REPEAT_FROM_TO_D(d, first, last, macro, data)
# endif
#
# define POTHOS_PP_REPEAT_FROM_TO_D POTHOS_PP_CAT(POTHOS_PP_REPEAT_FROM_TO_D_, POTHOS_PP_AUTO_REC(POTHOS_PP_REPEAT_P, 4))
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_REPEAT_FROM_TO_D_1(d, f, l, m, dt) POTHOS_PP_REPEAT_1(POTHOS_PP_SUB_D(d, l, f), POTHOS_PP_REPEAT_FROM_TO_M_1, (d, f, m, dt))
#    define POTHOS_PP_REPEAT_FROM_TO_D_2(d, f, l, m, dt) POTHOS_PP_REPEAT_2(POTHOS_PP_SUB_D(d, l, f), POTHOS_PP_REPEAT_FROM_TO_M_2, (d, f, m, dt))
#    define POTHOS_PP_REPEAT_FROM_TO_D_3(d, f, l, m, dt) POTHOS_PP_REPEAT_3(POTHOS_PP_SUB_D(d, l, f), POTHOS_PP_REPEAT_FROM_TO_M_3, (d, f, m, dt))
# else
#    define POTHOS_PP_REPEAT_FROM_TO_D_1(d, f, l, m, dt) POTHOS_PP_REPEAT_FROM_TO_D_1_I(d, f, l, m, dt)
#    define POTHOS_PP_REPEAT_FROM_TO_D_2(d, f, l, m, dt) POTHOS_PP_REPEAT_FROM_TO_D_2_I(d, f, l, m, dt)
#    define POTHOS_PP_REPEAT_FROM_TO_D_3(d, f, l, m, dt) POTHOS_PP_REPEAT_FROM_TO_D_3_I(d, f, l, m, dt)
#    define POTHOS_PP_REPEAT_FROM_TO_D_1_I(d, f, l, m, dt) POTHOS_PP_REPEAT_1(POTHOS_PP_SUB_D(d, l, f), POTHOS_PP_REPEAT_FROM_TO_M_1, (d, f, m, dt))
#    define POTHOS_PP_REPEAT_FROM_TO_D_2_I(d, f, l, m, dt) POTHOS_PP_REPEAT_2(POTHOS_PP_SUB_D(d, l, f), POTHOS_PP_REPEAT_FROM_TO_M_2, (d, f, m, dt))
#    define POTHOS_PP_REPEAT_FROM_TO_D_3_I(d, f, l, m, dt) POTHOS_PP_REPEAT_3(POTHOS_PP_SUB_D(d, l, f), POTHOS_PP_REPEAT_FROM_TO_M_3, (d, f, m, dt))
# endif
#
# if POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_STRICT()
#    define POTHOS_PP_REPEAT_FROM_TO_M_1(z, n, dfmd) POTHOS_PP_REPEAT_FROM_TO_M_1_IM(z, n, POTHOS_PP_TUPLE_REM_4 dfmd)
#    define POTHOS_PP_REPEAT_FROM_TO_M_2(z, n, dfmd) POTHOS_PP_REPEAT_FROM_TO_M_2_IM(z, n, POTHOS_PP_TUPLE_REM_4 dfmd)
#    define POTHOS_PP_REPEAT_FROM_TO_M_3(z, n, dfmd) POTHOS_PP_REPEAT_FROM_TO_M_3_IM(z, n, POTHOS_PP_TUPLE_REM_4 dfmd)
#    define POTHOS_PP_REPEAT_FROM_TO_M_1_IM(z, n, im) POTHOS_PP_REPEAT_FROM_TO_M_1_I(z, n, im)
#    define POTHOS_PP_REPEAT_FROM_TO_M_2_IM(z, n, im) POTHOS_PP_REPEAT_FROM_TO_M_2_I(z, n, im)
#    define POTHOS_PP_REPEAT_FROM_TO_M_3_IM(z, n, im) POTHOS_PP_REPEAT_FROM_TO_M_3_I(z, n, im)
# else
#    define POTHOS_PP_REPEAT_FROM_TO_M_1(z, n, dfmd) POTHOS_PP_REPEAT_FROM_TO_M_1_I(z, n, POTHOS_PP_TUPLE_ELEM(4, 0, dfmd), POTHOS_PP_TUPLE_ELEM(4, 1, dfmd), POTHOS_PP_TUPLE_ELEM(4, 2, dfmd), POTHOS_PP_TUPLE_ELEM(4, 3, dfmd))
#    define POTHOS_PP_REPEAT_FROM_TO_M_2(z, n, dfmd) POTHOS_PP_REPEAT_FROM_TO_M_2_I(z, n, POTHOS_PP_TUPLE_ELEM(4, 0, dfmd), POTHOS_PP_TUPLE_ELEM(4, 1, dfmd), POTHOS_PP_TUPLE_ELEM(4, 2, dfmd), POTHOS_PP_TUPLE_ELEM(4, 3, dfmd))
#    define POTHOS_PP_REPEAT_FROM_TO_M_3(z, n, dfmd) POTHOS_PP_REPEAT_FROM_TO_M_3_I(z, n, POTHOS_PP_TUPLE_ELEM(4, 0, dfmd), POTHOS_PP_TUPLE_ELEM(4, 1, dfmd), POTHOS_PP_TUPLE_ELEM(4, 2, dfmd), POTHOS_PP_TUPLE_ELEM(4, 3, dfmd))
# endif
#
# define POTHOS_PP_REPEAT_FROM_TO_M_1_I(z, n, d, f, m, dt) POTHOS_PP_REPEAT_FROM_TO_M_1_II(z, POTHOS_PP_ADD_D(d, n, f), m, dt)
# define POTHOS_PP_REPEAT_FROM_TO_M_2_I(z, n, d, f, m, dt) POTHOS_PP_REPEAT_FROM_TO_M_2_II(z, POTHOS_PP_ADD_D(d, n, f), m, dt)
# define POTHOS_PP_REPEAT_FROM_TO_M_3_I(z, n, d, f, m, dt) POTHOS_PP_REPEAT_FROM_TO_M_3_II(z, POTHOS_PP_ADD_D(d, n, f), m, dt)
#
# define POTHOS_PP_REPEAT_FROM_TO_M_1_II(z, n, m, dt) m(z, n, dt)
# define POTHOS_PP_REPEAT_FROM_TO_M_2_II(z, n, m, dt) m(z, n, dt)
# define POTHOS_PP_REPEAT_FROM_TO_M_3_II(z, n, m, dt) m(z, n, dt)
#
# endif
