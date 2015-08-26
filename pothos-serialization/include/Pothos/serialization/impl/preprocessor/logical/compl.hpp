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
# ifndef POTHOS_PREPROCESSOR_LOGICAL_COMPL_HPP
# define POTHOS_PREPROCESSOR_LOGICAL_COMPL_HPP
#
# include <Pothos/serialization/impl/preprocessor/config/config.hpp>
#
# /* POTHOS_PP_COMPL */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_MWCC()
#    define POTHOS_PP_COMPL(x) POTHOS_PP_COMPL_I(x)
# else
#    define POTHOS_PP_COMPL(x) POTHOS_PP_COMPL_OO((x))
#    define POTHOS_PP_COMPL_OO(par) POTHOS_PP_COMPL_I ## par
# endif
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_MSVC()
#    define POTHOS_PP_COMPL_I(x) POTHOS_PP_COMPL_ ## x
# else
#    define POTHOS_PP_COMPL_I(x) POTHOS_PP_COMPL_ID(POTHOS_PP_COMPL_ ## x)
#    define POTHOS_PP_COMPL_ID(id) id
# endif
#
# define POTHOS_PP_COMPL_0 1
# define POTHOS_PP_COMPL_1 0
#
# endif
