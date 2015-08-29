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
# ifndef POTHOS_PREPROCESSOR_CONTROL_EXPR_IIF_HPP
# define POTHOS_PREPROCESSOR_CONTROL_EXPR_IIF_HPP
#
# include <Pothos/serialization/impl/preprocessor/config/config.hpp>
#
# /* POTHOS_PP_EXPR_IIF */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_MWCC()
#    define POTHOS_PP_EXPR_IIF(bit, expr) POTHOS_PP_EXPR_IIF_I(bit, expr)
# else
#    define POTHOS_PP_EXPR_IIF(bit, expr) POTHOS_PP_EXPR_IIF_OO((bit, expr))
#    define POTHOS_PP_EXPR_IIF_OO(par) POTHOS_PP_EXPR_IIF_I ## par
# endif
#
# define POTHOS_PP_EXPR_IIF_I(bit, expr) POTHOS_PP_EXPR_IIF_ ## bit(expr)
#
# define POTHOS_PP_EXPR_IIF_0(expr)
# define POTHOS_PP_EXPR_IIF_1(expr) expr
#
# endif
