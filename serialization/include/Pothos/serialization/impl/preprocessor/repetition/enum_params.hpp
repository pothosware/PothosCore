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
# ifndef POTHOS_PREPROCESSOR_REPETITION_ENUM_PARAMS_HPP
# define POTHOS_PREPROCESSOR_REPETITION_ENUM_PARAMS_HPP
#
# include <Pothos/serialization/impl/preprocessor/config/config.hpp>
# include <Pothos/serialization/impl/preprocessor/punctuation/comma_if.hpp>
# include <Pothos/serialization/impl/preprocessor/repetition/repeat.hpp>
#
# /* POTHOS_PP_ENUM_PARAMS */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_ENUM_PARAMS(count, param) POTHOS_PP_REPEAT(count, POTHOS_PP_ENUM_PARAMS_M, param)
# else
#    define POTHOS_PP_ENUM_PARAMS(count, param) POTHOS_PP_ENUM_PARAMS_I(count, param)
#    define POTHOS_PP_ENUM_PARAMS_I(count, param) POTHOS_PP_REPEAT(count, POTHOS_PP_ENUM_PARAMS_M, param)
# endif
#
# define POTHOS_PP_ENUM_PARAMS_M(z, n, param) POTHOS_PP_COMMA_IF(n) param ## n
#
# /* POTHOS_PP_ENUM_PARAMS_Z */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_ENUM_PARAMS_Z(z, count, param) POTHOS_PP_REPEAT_ ## z(count, POTHOS_PP_ENUM_PARAMS_M, param)
# else
#    define POTHOS_PP_ENUM_PARAMS_Z(z, count, param) POTHOS_PP_ENUM_PARAMS_Z_I(z, count, param)
#    define POTHOS_PP_ENUM_PARAMS_Z_I(z, count, param) POTHOS_PP_REPEAT_ ## z(count, POTHOS_PP_ENUM_PARAMS_M, param)
# endif
#
# endif
