# /* Copyright (C) 2001
#  * Housemarque Oy
#  * http://www.housemarque.com
#  *
#  * Distributed under the Boost Software License, Version 1.0. (See
#  * accompanying file LICENSE_1_0.txt or copy at
#  * http://www.boost.org/LICENSE_1_0.txt)
#  *
#  * See http://www.boost.org for most recent version.
#  */
#
# /* Revised by Paul Mensonides (2002) */
#
# ifndef POTHOS_PREPROCESSOR_LIST_ADT_HPP
# define POTHOS_PREPROCESSOR_LIST_ADT_HPP
#
# include <Pothos/serialization/impl/preprocessor/config/config.hpp>
# include <Pothos/serialization/impl/preprocessor/detail/is_binary.hpp>
# include <Pothos/serialization/impl/preprocessor/logical/compl.hpp>
# include <Pothos/serialization/impl/preprocessor/tuple/eat.hpp>
#
# /* POTHOS_PP_LIST_CONS */
#
# define POTHOS_PP_LIST_CONS(head, tail) (head, tail)
#
# /* POTHOS_PP_LIST_NIL */
#
# define POTHOS_PP_LIST_NIL POTHOS_PP_NIL
#
# /* POTHOS_PP_LIST_FIRST */
#
# define POTHOS_PP_LIST_FIRST(list) POTHOS_PP_LIST_FIRST_D(list)
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_MWCC()
#    define POTHOS_PP_LIST_FIRST_D(list) POTHOS_PP_LIST_FIRST_I list
# else
#    define POTHOS_PP_LIST_FIRST_D(list) POTHOS_PP_LIST_FIRST_I ## list
# endif
#
# define POTHOS_PP_LIST_FIRST_I(head, tail) head
#
# /* POTHOS_PP_LIST_REST */
#
# define POTHOS_PP_LIST_REST(list) POTHOS_PP_LIST_REST_D(list)
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_MWCC()
#    define POTHOS_PP_LIST_REST_D(list) POTHOS_PP_LIST_REST_I list
# else
#    define POTHOS_PP_LIST_REST_D(list) POTHOS_PP_LIST_REST_I ## list
# endif
#
# define POTHOS_PP_LIST_REST_I(head, tail) tail
#
# /* POTHOS_PP_LIST_IS_CONS */
#
# if POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_BCC()
#    define POTHOS_PP_LIST_IS_CONS(list) POTHOS_PP_LIST_IS_CONS_D(list)
#    define POTHOS_PP_LIST_IS_CONS_D(list) POTHOS_PP_LIST_IS_CONS_ ## list
#    define POTHOS_PP_LIST_IS_CONS_(head, tail) 1
#    define POTHOS_PP_LIST_IS_CONS_POTHOS_PP_NIL 0
# else
#    define POTHOS_PP_LIST_IS_CONS(list) POTHOS_PP_IS_BINARY(list)
# endif
#
# /* POTHOS_PP_LIST_IS_NIL */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_BCC()
#    define POTHOS_PP_LIST_IS_NIL(list) POTHOS_PP_COMPL(POTHOS_PP_IS_BINARY(list))
# else
#    define POTHOS_PP_LIST_IS_NIL(list) POTHOS_PP_COMPL(POTHOS_PP_LIST_IS_CONS(list))
# endif
#
# endif
