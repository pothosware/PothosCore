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
# ifndef POTHOS_PREPROCESSOR_ARRAY_ELEM_HPP
# define POTHOS_PREPROCESSOR_ARRAY_ELEM_HPP
#
# include <Pothos/serialization/impl/preprocessor/array/data.hpp>
# include <Pothos/serialization/impl/preprocessor/array/size.hpp>
# include <Pothos/serialization/impl/preprocessor/config/config.hpp>
# include <Pothos/serialization/impl/preprocessor/tuple/elem.hpp>
#
# /* POTHOS_PP_ARRAY_ELEM */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_ARRAY_ELEM(i, array) POTHOS_PP_TUPLE_ELEM(POTHOS_PP_ARRAY_SIZE(array), i, POTHOS_PP_ARRAY_DATA(array))
# else
#    define POTHOS_PP_ARRAY_ELEM(i, array) POTHOS_PP_ARRAY_ELEM_I(i, array)
#    define POTHOS_PP_ARRAY_ELEM_I(i, array) POTHOS_PP_TUPLE_ELEM(POTHOS_PP_ARRAY_SIZE(array), i, POTHOS_PP_ARRAY_DATA(array))
# endif
#
# endif
