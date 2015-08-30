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
# ifndef POTHOS_PREPROCESSOR_PUNCTUATION_COMMA_IF_HPP
# define POTHOS_PREPROCESSOR_PUNCTUATION_COMMA_IF_HPP
#
# include <Pothos/serialization/impl/preprocessor/config/config.hpp>
# include <Pothos/serialization/impl/preprocessor/control/if.hpp>
# include <Pothos/serialization/impl/preprocessor/facilities/empty.hpp>
# include <Pothos/serialization/impl/preprocessor/punctuation/comma.hpp>
#
# /* POTHOS_PP_COMMA_IF */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_COMMA_IF(cond) POTHOS_PP_IF(cond, POTHOS_PP_COMMA, POTHOS_PP_EMPTY)()
# else
#    define POTHOS_PP_COMMA_IF(cond) POTHOS_PP_COMMA_IF_I(cond)
#    define POTHOS_PP_COMMA_IF_I(cond) POTHOS_PP_IF(cond, POTHOS_PP_COMMA, POTHOS_PP_EMPTY)()
# endif
#
# endif
