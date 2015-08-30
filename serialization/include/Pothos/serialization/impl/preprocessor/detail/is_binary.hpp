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
# ifndef POTHOS_PREPROCESSOR_DETAIL_IS_BINARY_HPP
# define POTHOS_PREPROCESSOR_DETAIL_IS_BINARY_HPP
#
# include <Pothos/serialization/impl/preprocessor/config/config.hpp>
# include <Pothos/serialization/impl/preprocessor/detail/check.hpp>
#
# /* POTHOS_PP_IS_BINARY */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_IS_BINARY(x) POTHOS_PP_CHECK(x, POTHOS_PP_IS_BINARY_CHECK)
# else
#    define POTHOS_PP_IS_BINARY(x) POTHOS_PP_IS_BINARY_I(x)
#    define POTHOS_PP_IS_BINARY_I(x) POTHOS_PP_CHECK(x, POTHOS_PP_IS_BINARY_CHECK)
# endif
#
# define POTHOS_PP_IS_BINARY_CHECK(a, b) 1
# define POTHOS_PP_CHECK_RESULT_POTHOS_PP_IS_BINARY_CHECK 0, POTHOS_PP_NIL
#
# endif
