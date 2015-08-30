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
# ifndef POTHOS_PREPROCESSOR_ARRAY_DATA_HPP
# define POTHOS_PREPROCESSOR_ARRAY_DATA_HPP
#
# include <Pothos/serialization/impl/preprocessor/config/config.hpp>
# include <Pothos/serialization/impl/preprocessor/tuple/elem.hpp>
#
# /* POTHOS_PP_ARRAY_DATA */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_ARRAY_DATA(array) POTHOS_PP_TUPLE_ELEM(2, 1, array)
# else
#    define POTHOS_PP_ARRAY_DATA(array) POTHOS_PP_ARRAY_DATA_I(array)
#    define POTHOS_PP_ARRAY_DATA_I(array) POTHOS_PP_ARRAY_DATA_II array
#    define POTHOS_PP_ARRAY_DATA_II(size, data) data
# endif
#
# endif
