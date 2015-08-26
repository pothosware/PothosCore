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
# ifndef POTHOS_PREPROCESSOR_DEBUG_ERROR_HPP
# define POTHOS_PREPROCESSOR_DEBUG_ERROR_HPP
#
# include <Pothos/serialization/impl/preprocessor/cat.hpp>
# include <Pothos/serialization/impl/preprocessor/config/config.hpp>
#
# /* POTHOS_PP_ERROR */
#
# if POTHOS_PP_CONFIG_ERRORS
#    define POTHOS_PP_ERROR(code) POTHOS_PP_CAT(POTHOS_PP_ERROR_, code)
# endif
#
# define POTHOS_PP_ERROR_0x0000 POTHOS_PP_ERROR(0x0000, POTHOS_PP_INDEX_OUT_OF_BOUNDS)
# define POTHOS_PP_ERROR_0x0001 POTHOS_PP_ERROR(0x0001, POTHOS_PP_WHILE_OVERFLOW)
# define POTHOS_PP_ERROR_0x0002 POTHOS_PP_ERROR(0x0002, POTHOS_PP_FOR_OVERFLOW)
# define POTHOS_PP_ERROR_0x0003 POTHOS_PP_ERROR(0x0003, POTHOS_PP_REPEAT_OVERFLOW)
# define POTHOS_PP_ERROR_0x0004 POTHOS_PP_ERROR(0x0004, POTHOS_PP_LIST_FOLD_OVERFLOW)
# define POTHOS_PP_ERROR_0x0005 POTHOS_PP_ERROR(0x0005, POTHOS_PP_SEQ_FOLD_OVERFLOW)
# define POTHOS_PP_ERROR_0x0006 POTHOS_PP_ERROR(0x0006, POTHOS_PP_ARITHMETIC_OVERFLOW)
# define POTHOS_PP_ERROR_0x0007 POTHOS_PP_ERROR(0x0007, POTHOS_PP_DIVISION_BY_ZERO)
#
# endif
