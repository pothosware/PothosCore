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
# ifndef POTHOS_PREPROCESSOR_LIST_LIST_FOR_EACH_I_HPP
# define POTHOS_PREPROCESSOR_LIST_LIST_FOR_EACH_I_HPP
#
# include <Pothos/serialization/impl/preprocessor/arithmetic/inc.hpp>
# include <Pothos/serialization/impl/preprocessor/config/config.hpp>
# include <Pothos/serialization/impl/preprocessor/list/adt.hpp>
# include <Pothos/serialization/impl/preprocessor/repetition/for.hpp>
# include <Pothos/serialization/impl/preprocessor/tuple/elem.hpp>
# include <Pothos/serialization/impl/preprocessor/tuple/rem.hpp>
#
# /* POTHOS_PP_LIST_FOR_EACH_I */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG() && ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_MSVC()
#    define POTHOS_PP_LIST_FOR_EACH_I(macro, data, list) POTHOS_PP_FOR((macro, data, list, 0), POTHOS_PP_LIST_FOR_EACH_I_P, POTHOS_PP_LIST_FOR_EACH_I_O, POTHOS_PP_LIST_FOR_EACH_I_M)
# else
#    define POTHOS_PP_LIST_FOR_EACH_I(macro, data, list) POTHOS_PP_LIST_FOR_EACH_I_I(macro, data, list)
#    define POTHOS_PP_LIST_FOR_EACH_I_I(macro, data, list) POTHOS_PP_FOR((macro, data, list, 0), POTHOS_PP_LIST_FOR_EACH_I_P, POTHOS_PP_LIST_FOR_EACH_I_O, POTHOS_PP_LIST_FOR_EACH_I_M)
# endif
#
# if POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_STRICT()
#    define POTHOS_PP_LIST_FOR_EACH_I_P(r, x) POTHOS_PP_LIST_FOR_EACH_I_P_D x
#    define POTHOS_PP_LIST_FOR_EACH_I_P_D(m, d, l, i) POTHOS_PP_LIST_IS_CONS(l)
# else
#    define POTHOS_PP_LIST_FOR_EACH_I_P(r, x) POTHOS_PP_LIST_IS_CONS(POTHOS_PP_TUPLE_ELEM(4, 2, x))
# endif
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_MWCC()
#    define POTHOS_PP_LIST_FOR_EACH_I_O(r, x) POTHOS_PP_LIST_FOR_EACH_I_O_D x
#    define POTHOS_PP_LIST_FOR_EACH_I_O_D(m, d, l, i) (m, d, POTHOS_PP_LIST_REST(l), POTHOS_PP_INC(i))
# else
#    define POTHOS_PP_LIST_FOR_EACH_I_O(r, x) (POTHOS_PP_TUPLE_ELEM(4, 0, x), POTHOS_PP_TUPLE_ELEM(4, 1, x), POTHOS_PP_LIST_REST(POTHOS_PP_TUPLE_ELEM(4, 2, x)), POTHOS_PP_INC(POTHOS_PP_TUPLE_ELEM(4, 3, x)))
# endif
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_LIST_FOR_EACH_I_M(r, x) POTHOS_PP_LIST_FOR_EACH_I_M_D(r, POTHOS_PP_TUPLE_ELEM(4, 0, x), POTHOS_PP_TUPLE_ELEM(4, 1, x), POTHOS_PP_TUPLE_ELEM(4, 2, x), POTHOS_PP_TUPLE_ELEM(4, 3, x))
# else
#    define POTHOS_PP_LIST_FOR_EACH_I_M(r, x) POTHOS_PP_LIST_FOR_EACH_I_M_I(r, POTHOS_PP_TUPLE_REM_4 x)
#    define POTHOS_PP_LIST_FOR_EACH_I_M_I(r, x_e) POTHOS_PP_LIST_FOR_EACH_I_M_D(r, x_e)
# endif
#
# define POTHOS_PP_LIST_FOR_EACH_I_M_D(r, m, d, l, i) m(r, d, i, POTHOS_PP_LIST_FIRST(l))
#
# /* POTHOS_PP_LIST_FOR_EACH_I_R */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_LIST_FOR_EACH_I_R(r, macro, data, list) POTHOS_PP_FOR_ ## r((macro, data, list, 0), POTHOS_PP_LIST_FOR_EACH_I_P, POTHOS_PP_LIST_FOR_EACH_I_O, POTHOS_PP_LIST_FOR_EACH_I_M)
# else
#    define POTHOS_PP_LIST_FOR_EACH_I_R(r, macro, data, list) POTHOS_PP_LIST_FOR_EACH_I_R_I(r, macro, data, list)
#    define POTHOS_PP_LIST_FOR_EACH_I_R_I(r, macro, data, list) POTHOS_PP_FOR_ ## r((macro, data, list, 0), POTHOS_PP_LIST_FOR_EACH_I_P, POTHOS_PP_LIST_FOR_EACH_I_O, POTHOS_PP_LIST_FOR_EACH_I_M)
# endif
#
# endif
