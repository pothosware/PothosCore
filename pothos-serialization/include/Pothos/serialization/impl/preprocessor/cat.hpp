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
# ifndef POTHOS_PREPROCESSOR_CAT_HPP
# define POTHOS_PREPROCESSOR_CAT_HPP
#
# include <Pothos/serialization/impl/preprocessor/config/config.hpp>
#
# /* POTHOS_PP_CAT */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_MWCC()
#    define POTHOS_PP_CAT(a, b) POTHOS_PP_CAT_I(a, b)
# else
#    define POTHOS_PP_CAT(a, b) POTHOS_PP_CAT_OO((a, b))
#    define POTHOS_PP_CAT_OO(par) POTHOS_PP_CAT_I ## par
# endif
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_MSVC()
#    define POTHOS_PP_CAT_I(a, b) a ## b
# else
#    define POTHOS_PP_CAT_I(a, b) POTHOS_PP_CAT_II(~, a ## b)
#    define POTHOS_PP_CAT_II(p, res) res
# endif
#
# endif
