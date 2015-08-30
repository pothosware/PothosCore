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
# ifndef POTHOS_PREPROCESSOR_CONTROL_IIF_HPP
# define POTHOS_PREPROCESSOR_CONTROL_IIF_HPP
#
# include <Pothos/serialization/impl/preprocessor/config/config.hpp>
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_MWCC()
#    define POTHOS_PP_IIF(bit, t, f) POTHOS_PP_IIF_I(bit, t, f)
# else
#    define POTHOS_PP_IIF(bit, t, f) POTHOS_PP_IIF_OO((bit, t, f))
#    define POTHOS_PP_IIF_OO(par) POTHOS_PP_IIF_I ## par
# endif
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_MSVC()
#    define POTHOS_PP_IIF_I(bit, t, f) POTHOS_PP_IIF_ ## bit(t, f)
# else
#    define POTHOS_PP_IIF_I(bit, t, f) POTHOS_PP_IIF_II(POTHOS_PP_IIF_ ## bit(t, f))
#    define POTHOS_PP_IIF_II(id) id
# endif
#
# define POTHOS_PP_IIF_0(t, f) f
# define POTHOS_PP_IIF_1(t, f) t
#
# endif
