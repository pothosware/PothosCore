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
# ifndef POTHOS_PREPROCESSOR_LOGICAL_NOT_HPP
# define POTHOS_PREPROCESSOR_LOGICAL_NOT_HPP
#
# include <Pothos/serialization/impl/preprocessor/config/config.hpp>
# include <Pothos/serialization/impl/preprocessor/logical/bool.hpp>
# include <Pothos/serialization/impl/preprocessor/logical/compl.hpp>
#
# /* POTHOS_PP_NOT */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_NOT(x) POTHOS_PP_COMPL(POTHOS_PP_BOOL(x))
# else
#    define POTHOS_PP_NOT(x) POTHOS_PP_NOT_I(x)
#    define POTHOS_PP_NOT_I(x) POTHOS_PP_COMPL(POTHOS_PP_BOOL(x))
# endif
#
# endif
