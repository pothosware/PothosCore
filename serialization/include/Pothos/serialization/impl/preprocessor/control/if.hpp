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
# ifndef POTHOS_PREPROCESSOR_CONTROL_IF_HPP
# define POTHOS_PREPROCESSOR_CONTROL_IF_HPP
#
# include <Pothos/serialization/impl/preprocessor/config/config.hpp>
# include <Pothos/serialization/impl/preprocessor/control/iif.hpp>
# include <Pothos/serialization/impl/preprocessor/logical/bool.hpp>
#
# /* POTHOS_PP_IF */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_IF(cond, t, f) POTHOS_PP_IIF(POTHOS_PP_BOOL(cond), t, f)
# else
#    define POTHOS_PP_IF(cond, t, f) POTHOS_PP_IF_I(cond, t, f)
#    define POTHOS_PP_IF_I(cond, t, f) POTHOS_PP_IIF(POTHOS_PP_BOOL(cond), t, f)
# endif
#
# endif
