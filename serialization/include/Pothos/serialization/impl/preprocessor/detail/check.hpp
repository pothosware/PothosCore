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
# ifndef POTHOS_PREPROCESSOR_DETAIL_CHECK_HPP
# define POTHOS_PREPROCESSOR_DETAIL_CHECK_HPP
#
# include <Pothos/serialization/impl/preprocessor/cat.hpp>
# include <Pothos/serialization/impl/preprocessor/config/config.hpp>
#
# /* POTHOS_PP_CHECK */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_MWCC()
#    define POTHOS_PP_CHECK(x, type) POTHOS_PP_CHECK_D(x, type)
# else
#    define POTHOS_PP_CHECK(x, type) POTHOS_PP_CHECK_OO((x, type))
#    define POTHOS_PP_CHECK_OO(par) POTHOS_PP_CHECK_D ## par
# endif
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_MSVC() && ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_DMC()
#    define POTHOS_PP_CHECK_D(x, type) POTHOS_PP_CHECK_1(POTHOS_PP_CAT(POTHOS_PP_CHECK_RESULT_, type x))
#    define POTHOS_PP_CHECK_1(chk) POTHOS_PP_CHECK_2(chk)
#    define POTHOS_PP_CHECK_2(res, _) res
# elif POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_MSVC()
#    define POTHOS_PP_CHECK_D(x, type) POTHOS_PP_CHECK_1(type x)
#    define POTHOS_PP_CHECK_1(chk) POTHOS_PP_CHECK_2(chk)
#    define POTHOS_PP_CHECK_2(chk) POTHOS_PP_CHECK_3((POTHOS_PP_CHECK_RESULT_ ## chk))
#    define POTHOS_PP_CHECK_3(im) POTHOS_PP_CHECK_5(POTHOS_PP_CHECK_4 im)
#    define POTHOS_PP_CHECK_4(res, _) res
#    define POTHOS_PP_CHECK_5(res) res
# else /* DMC */
#    define POTHOS_PP_CHECK_D(x, type) POTHOS_PP_CHECK_OO((type x))
#    define POTHOS_PP_CHECK_OO(par) POTHOS_PP_CHECK_0 ## par
#    define POTHOS_PP_CHECK_0(chk) POTHOS_PP_CHECK_1(POTHOS_PP_CAT(POTHOS_PP_CHECK_RESULT_, chk))
#    define POTHOS_PP_CHECK_1(chk) POTHOS_PP_CHECK_2(chk)
#    define POTHOS_PP_CHECK_2(res, _) res
# endif
#
# define POTHOS_PP_CHECK_RESULT_1 1, POTHOS_PP_NIL
#
# endif
