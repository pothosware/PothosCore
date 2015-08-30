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
# ifndef POTHOS_PREPROCESSOR_LOGICAL_BITAND_HPP
# define POTHOS_PREPROCESSOR_LOGICAL_BITAND_HPP
#
# include <Pothos/serialization/impl/preprocessor/config/config.hpp>
#
# /* POTHOS_PP_BITAND */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_MWCC()
#    define POTHOS_PP_BITAND(x, y) POTHOS_PP_BITAND_I(x, y)
# else
#    define POTHOS_PP_BITAND(x, y) POTHOS_PP_BITAND_OO((x, y))
#    define POTHOS_PP_BITAND_OO(par) POTHOS_PP_BITAND_I ## par
# endif
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_MSVC()
#    define POTHOS_PP_BITAND_I(x, y) POTHOS_PP_BITAND_ ## x ## y
# else
#    define POTHOS_PP_BITAND_I(x, y) POTHOS_PP_BITAND_ID(POTHOS_PP_BITAND_ ## x ## y)
#    define POTHOS_PP_BITAND_ID(res) res
# endif
#
# define POTHOS_PP_BITAND_00 0
# define POTHOS_PP_BITAND_01 0
# define POTHOS_PP_BITAND_10 0
# define POTHOS_PP_BITAND_11 1
#
# endif
