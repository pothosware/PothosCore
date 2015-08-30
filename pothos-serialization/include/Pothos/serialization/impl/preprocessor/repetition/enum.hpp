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
# ifndef POTHOS_PREPROCESSOR_REPETITION_ENUM_HPP
# define POTHOS_PREPROCESSOR_REPETITION_ENUM_HPP
#
# include <Pothos/serialization/impl/preprocessor/cat.hpp>
# include <Pothos/serialization/impl/preprocessor/config/config.hpp>
# include <Pothos/serialization/impl/preprocessor/debug/error.hpp>
# include <Pothos/serialization/impl/preprocessor/detail/auto_rec.hpp>
# include <Pothos/serialization/impl/preprocessor/punctuation/comma_if.hpp>
# include <Pothos/serialization/impl/preprocessor/repetition/repeat.hpp>
# include <Pothos/serialization/impl/preprocessor/tuple/elem.hpp>
# include <Pothos/serialization/impl/preprocessor/tuple/rem.hpp>
#
# /* POTHOS_PP_ENUM */
#
# if 0
#    define POTHOS_PP_ENUM(count, macro, data)
# endif
#
# define POTHOS_PP_ENUM POTHOS_PP_CAT(POTHOS_PP_ENUM_, POTHOS_PP_AUTO_REC(POTHOS_PP_REPEAT_P, 4))
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_ENUM_1(c, m, d) POTHOS_PP_REPEAT_1(c, POTHOS_PP_ENUM_M_1, (m, d))
#    define POTHOS_PP_ENUM_2(c, m, d) POTHOS_PP_REPEAT_2(c, POTHOS_PP_ENUM_M_2, (m, d))
#    define POTHOS_PP_ENUM_3(c, m, d) POTHOS_PP_REPEAT_3(c, POTHOS_PP_ENUM_M_3, (m, d))
# else
#    define POTHOS_PP_ENUM_1(c, m, d) POTHOS_PP_ENUM_1_I(c, m, d)
#    define POTHOS_PP_ENUM_2(c, m, d) POTHOS_PP_ENUM_2_I(c, m, d)
#    define POTHOS_PP_ENUM_3(c, m, d) POTHOS_PP_ENUM_3_I(c, m, d)
#    define POTHOS_PP_ENUM_1_I(c, m, d) POTHOS_PP_REPEAT_1(c, POTHOS_PP_ENUM_M_1, (m, d))
#    define POTHOS_PP_ENUM_2_I(c, m, d) POTHOS_PP_REPEAT_2(c, POTHOS_PP_ENUM_M_2, (m, d))
#    define POTHOS_PP_ENUM_3_I(c, m, d) POTHOS_PP_REPEAT_3(c, POTHOS_PP_ENUM_M_3, (m, d))
# endif
#
# define POTHOS_PP_ENUM_4(c, m, d) POTHOS_PP_ERROR(0x0003)
#
# if POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_STRICT()
#    define POTHOS_PP_ENUM_M_1(z, n, md) POTHOS_PP_ENUM_M_1_IM(z, n, POTHOS_PP_TUPLE_REM_2 md)
#    define POTHOS_PP_ENUM_M_2(z, n, md) POTHOS_PP_ENUM_M_2_IM(z, n, POTHOS_PP_TUPLE_REM_2 md)
#    define POTHOS_PP_ENUM_M_3(z, n, md) POTHOS_PP_ENUM_M_3_IM(z, n, POTHOS_PP_TUPLE_REM_2 md)
#    define POTHOS_PP_ENUM_M_1_IM(z, n, im) POTHOS_PP_ENUM_M_1_I(z, n, im)
#    define POTHOS_PP_ENUM_M_2_IM(z, n, im) POTHOS_PP_ENUM_M_2_I(z, n, im)
#    define POTHOS_PP_ENUM_M_3_IM(z, n, im) POTHOS_PP_ENUM_M_3_I(z, n, im)
# else
#    define POTHOS_PP_ENUM_M_1(z, n, md) POTHOS_PP_ENUM_M_1_I(z, n, POTHOS_PP_TUPLE_ELEM(2, 0, md), POTHOS_PP_TUPLE_ELEM(2, 1, md))
#    define POTHOS_PP_ENUM_M_2(z, n, md) POTHOS_PP_ENUM_M_2_I(z, n, POTHOS_PP_TUPLE_ELEM(2, 0, md), POTHOS_PP_TUPLE_ELEM(2, 1, md))
#    define POTHOS_PP_ENUM_M_3(z, n, md) POTHOS_PP_ENUM_M_3_I(z, n, POTHOS_PP_TUPLE_ELEM(2, 0, md), POTHOS_PP_TUPLE_ELEM(2, 1, md))
# endif
#
# define POTHOS_PP_ENUM_M_1_I(z, n, m, d) POTHOS_PP_COMMA_IF(n) m(z, n, d)
# define POTHOS_PP_ENUM_M_2_I(z, n, m, d) POTHOS_PP_COMMA_IF(n) m(z, n, d)
# define POTHOS_PP_ENUM_M_3_I(z, n, m, d) POTHOS_PP_COMMA_IF(n) m(z, n, d)
#
# endif
