# /* **************************************************************************
#  *                                                                          *
#  *     (C) Copyright Paul Mensonides 2002.
#  *     Distributed under the Boost Software License, Version 1.0. (See
#  *     accompanying file LICENSE_1_0.txt or copy at
#  *     http://www.boost.org/LICENSE_1_0.txt)
#  *                                                                          *
#  ************************************************************************** */
#
# /* See http://www.boost.org for most recent version. */
#
# ifndef POTHOS_PREPROCESSOR_REPETITION_ENUM_BINARY_PARAMS_HPP
# define POTHOS_PREPROCESSOR_REPETITION_ENUM_BINARY_PARAMS_HPP
#
# include <Pothos/serialization/impl/preprocessor/cat.hpp>
# include <Pothos/serialization/impl/preprocessor/config/config.hpp>
# include <Pothos/serialization/impl/preprocessor/punctuation/comma_if.hpp>
# include <Pothos/serialization/impl/preprocessor/repetition/repeat.hpp>
# include <Pothos/serialization/impl/preprocessor/tuple/elem.hpp>
# include <Pothos/serialization/impl/preprocessor/tuple/rem.hpp>
#
# /* POTHOS_PP_ENUM_BINARY_PARAMS */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_ENUM_BINARY_PARAMS(count, p1, p2) POTHOS_PP_REPEAT(count, POTHOS_PP_ENUM_BINARY_PARAMS_M, (p1, p2))
# else
#    define POTHOS_PP_ENUM_BINARY_PARAMS(count, p1, p2) POTHOS_PP_ENUM_BINARY_PARAMS_I(count, p1, p2)
#    define POTHOS_PP_ENUM_BINARY_PARAMS_I(count, p1, p2) POTHOS_PP_REPEAT(count, POTHOS_PP_ENUM_BINARY_PARAMS_M, (p1, p2))
# endif
#
# if POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_STRICT()
#    define POTHOS_PP_ENUM_BINARY_PARAMS_M(z, n, pp) POTHOS_PP_ENUM_BINARY_PARAMS_M_IM(z, n, POTHOS_PP_TUPLE_REM_2 pp)
#    define POTHOS_PP_ENUM_BINARY_PARAMS_M_IM(z, n, im) POTHOS_PP_ENUM_BINARY_PARAMS_M_I(z, n, im)
# else
#    define POTHOS_PP_ENUM_BINARY_PARAMS_M(z, n, pp) POTHOS_PP_ENUM_BINARY_PARAMS_M_I(z, n, POTHOS_PP_TUPLE_ELEM(2, 0, pp), POTHOS_PP_TUPLE_ELEM(2, 1, pp))
# endif
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_MSVC()
#    define POTHOS_PP_ENUM_BINARY_PARAMS_M_I(z, n, p1, p2) POTHOS_PP_ENUM_BINARY_PARAMS_M_II(z, n, p1, p2)
#    define POTHOS_PP_ENUM_BINARY_PARAMS_M_II(z, n, p1, p2) POTHOS_PP_COMMA_IF(n) p1 ## n p2 ## n
# else
#    define POTHOS_PP_ENUM_BINARY_PARAMS_M_I(z, n, p1, p2) POTHOS_PP_COMMA_IF(n) POTHOS_PP_CAT(p1, n) POTHOS_PP_CAT(p2, n)
# endif
#
# /* POTHOS_PP_ENUM_BINARY_PARAMS_Z */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_ENUM_BINARY_PARAMS_Z(z, count, p1, p2) POTHOS_PP_REPEAT_ ## z(count, POTHOS_PP_ENUM_BINARY_PARAMS_M, (p1, p2))
# else
#    define POTHOS_PP_ENUM_BINARY_PARAMS_Z(z, count, p1, p2) POTHOS_PP_ENUM_BINARY_PARAMS_Z_I(z, count, p1, p2)
#    define POTHOS_PP_ENUM_BINARY_PARAMS_Z_I(z, count, p1, p2) POTHOS_PP_REPEAT_ ## z(count, POTHOS_PP_ENUM_BINARY_PARAMS_M, (p1, p2))
# endif
#
# endif
