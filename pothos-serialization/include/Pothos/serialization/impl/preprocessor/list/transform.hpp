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
# ifndef POTHOS_PREPROCESSOR_LIST_TRANSFORM_HPP
# define POTHOS_PREPROCESSOR_LIST_TRANSFORM_HPP
#
# include <Pothos/serialization/impl/preprocessor/config/config.hpp>
# include <Pothos/serialization/impl/preprocessor/list/fold_right.hpp>
# include <Pothos/serialization/impl/preprocessor/tuple/elem.hpp>
# include <Pothos/serialization/impl/preprocessor/tuple/rem.hpp>
#
# /* POTHOS_PP_LIST_TRANSFORM */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_LIST_TRANSFORM(op, data, list) POTHOS_PP_TUPLE_ELEM(3, 2, POTHOS_PP_LIST_FOLD_RIGHT(POTHOS_PP_LIST_TRANSFORM_O, (op, data, POTHOS_PP_NIL), list))
# else
#    define POTHOS_PP_LIST_TRANSFORM(op, data, list) POTHOS_PP_LIST_TRANSFORM_I(op, data, list)
#    define POTHOS_PP_LIST_TRANSFORM_I(op, data, list) POTHOS_PP_TUPLE_ELEM(3, 2, POTHOS_PP_LIST_FOLD_RIGHT(POTHOS_PP_LIST_TRANSFORM_O, (op, data, POTHOS_PP_NIL), list))
# endif
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_LIST_TRANSFORM_O(d, odr, elem) POTHOS_PP_LIST_TRANSFORM_O_D(d, POTHOS_PP_TUPLE_ELEM(3, 0, odr), POTHOS_PP_TUPLE_ELEM(3, 1, odr), POTHOS_PP_TUPLE_ELEM(3, 2, odr), elem)
# else
#    define POTHOS_PP_LIST_TRANSFORM_O(d, odr, elem) POTHOS_PP_LIST_TRANSFORM_O_I(d, POTHOS_PP_TUPLE_REM_3 odr, elem)
#    define POTHOS_PP_LIST_TRANSFORM_O_I(d, im, elem) POTHOS_PP_LIST_TRANSFORM_O_D(d, im, elem)
# endif
#
# define POTHOS_PP_LIST_TRANSFORM_O_D(d, op, data, res, elem) (op, data, (op(d, data, elem), res))
#
# /* POTHOS_PP_LIST_TRANSFORM_D */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_LIST_TRANSFORM_D(d, op, data, list) POTHOS_PP_TUPLE_ELEM(3, 2, POTHOS_PP_LIST_FOLD_RIGHT_ ## d(POTHOS_PP_LIST_TRANSFORM_O, (op, data, POTHOS_PP_NIL), list))
# else
#    define POTHOS_PP_LIST_TRANSFORM_D(d, op, data, list) POTHOS_PP_LIST_TRANSFORM_D_I(d, op, data, list)
#    define POTHOS_PP_LIST_TRANSFORM_D_I(d, op, data, list) POTHOS_PP_TUPLE_ELEM(3, 2, POTHOS_PP_LIST_FOLD_RIGHT_ ## d(POTHOS_PP_LIST_TRANSFORM_O, (op, data, POTHOS_PP_NIL), list))
# endif
#
# endif
