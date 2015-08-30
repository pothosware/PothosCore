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
# ifndef POTHOS_PREPROCESSOR_STRINGIZE_HPP
# define POTHOS_PREPROCESSOR_STRINGIZE_HPP
#
# include <Pothos/serialization/impl/preprocessor/config/config.hpp>
#
# /* POTHOS_PP_STRINGIZE */
#
# if POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_MSVC()
#    define POTHOS_PP_STRINGIZE(text) POTHOS_PP_STRINGIZE_A((text))
#    define POTHOS_PP_STRINGIZE_A(arg) POTHOS_PP_STRINGIZE_I arg
# elif POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_MWCC()
#    define POTHOS_PP_STRINGIZE(text) POTHOS_PP_STRINGIZE_OO((text))
#    define POTHOS_PP_STRINGIZE_OO(par) POTHOS_PP_STRINGIZE_I ## par
# else
#    define POTHOS_PP_STRINGIZE(text) POTHOS_PP_STRINGIZE_I(text)
# endif
#
# define POTHOS_PP_STRINGIZE_I(text) #text
#
# endif
