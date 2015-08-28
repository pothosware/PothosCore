# /* **************************************************************************
#  *                                                                          *
#  *     (C) Copyright Paul Mensonides 2011.                                  *
#  *     (C) Copyright Edward Diener 2011.                                    *
#  *     Distributed under the Boost Software License, Version 1.0. (See      *
#  *     accompanying file LICENSE_1_0.txt or copy at                         *
#  *     http://www.boost.org/LICENSE_1_0.txt)                                *
#  *                                                                          *
#  ************************************************************************** */
#
# /* See http://www.boost.org for most recent version. */
#
# ifndef POTHOS_PREPROCESSOR_FACILITIES_OVERLOAD_HPP
# define POTHOS_PREPROCESSOR_FACILITIES_OVERLOAD_HPP
#
# include <Pothos/serialization/impl/preprocessor/cat.hpp>
# include <Pothos/serialization/impl/preprocessor/variadic/size.hpp>
#
# /* POTHOS_PP_OVERLOAD */
#
# if POTHOS_PP_VARIADICS
#    define POTHOS_PP_OVERLOAD(prefix, ...) POTHOS_PP_CAT(prefix, POTHOS_PP_VARIADIC_SIZE(__VA_ARGS__))
# endif
#
# endif
