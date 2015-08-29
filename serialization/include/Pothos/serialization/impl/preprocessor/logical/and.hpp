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
# ifndef POTHOS_PREPROCESSOR_LOGICAL_AND_HPP
# define POTHOS_PREPROCESSOR_LOGICAL_AND_HPP
#
# include <Pothos/serialization/impl/preprocessor/config/config.hpp>
# include <Pothos/serialization/impl/preprocessor/logical/bool.hpp>
# include <Pothos/serialization/impl/preprocessor/logical/bitand.hpp>
#
# /* POTHOS_PP_AND */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_AND(p, q) POTHOS_PP_BITAND(POTHOS_PP_BOOL(p), POTHOS_PP_BOOL(q))
# else
#    define POTHOS_PP_AND(p, q) POTHOS_PP_AND_I(p, q)
#    define POTHOS_PP_AND_I(p, q) POTHOS_PP_BITAND(POTHOS_PP_BOOL(p), POTHOS_PP_BOOL(q))
# endif
#
# endif
