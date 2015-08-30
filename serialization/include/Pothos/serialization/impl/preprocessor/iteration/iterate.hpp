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
# ifndef POTHOS_PREPROCESSOR_ITERATION_ITERATE_HPP
# define POTHOS_PREPROCESSOR_ITERATION_ITERATE_HPP
#
# include <Pothos/serialization/impl/preprocessor/arithmetic/dec.hpp>
# include <Pothos/serialization/impl/preprocessor/arithmetic/inc.hpp>
# include <Pothos/serialization/impl/preprocessor/array/elem.hpp>
# include <Pothos/serialization/impl/preprocessor/array/size.hpp>
# include <Pothos/serialization/impl/preprocessor/cat.hpp>
# include <Pothos/serialization/impl/preprocessor/slot/slot.hpp>
# include <Pothos/serialization/impl/preprocessor/tuple/elem.hpp>
#
# /* POTHOS_PP_ITERATION_DEPTH */
#
# define POTHOS_PP_ITERATION_DEPTH() 0
#
# /* POTHOS_PP_ITERATION */
#
# define POTHOS_PP_ITERATION() POTHOS_PP_CAT(POTHOS_PP_ITERATION_, POTHOS_PP_ITERATION_DEPTH())
#
# /* POTHOS_PP_ITERATION_START && POTHOS_PP_ITERATION_FINISH */
#
# define POTHOS_PP_ITERATION_START() POTHOS_PP_CAT(POTHOS_PP_ITERATION_START_, POTHOS_PP_ITERATION_DEPTH())
# define POTHOS_PP_ITERATION_FINISH() POTHOS_PP_CAT(POTHOS_PP_ITERATION_FINISH_, POTHOS_PP_ITERATION_DEPTH())
#
# /* POTHOS_PP_ITERATION_FLAGS */
#
# define POTHOS_PP_ITERATION_FLAGS() (POTHOS_PP_CAT(POTHOS_PP_ITERATION_FLAGS_, POTHOS_PP_ITERATION_DEPTH())())
#
# /* POTHOS_PP_FRAME_ITERATION */
#
# define POTHOS_PP_FRAME_ITERATION(i) POTHOS_PP_CAT(POTHOS_PP_ITERATION_, i)
#
# /* POTHOS_PP_FRAME_START && POTHOS_PP_FRAME_FINISH */
#
# define POTHOS_PP_FRAME_START(i) POTHOS_PP_CAT(POTHOS_PP_ITERATION_START_, i)
# define POTHOS_PP_FRAME_FINISH(i) POTHOS_PP_CAT(POTHOS_PP_ITERATION_FINISH_, i)
#
# /* POTHOS_PP_FRAME_FLAGS */
#
# define POTHOS_PP_FRAME_FLAGS(i) (POTHOS_PP_CAT(POTHOS_PP_ITERATION_FLAGS_, i)())
#
# /* POTHOS_PP_RELATIVE_ITERATION */
#
# define POTHOS_PP_RELATIVE_ITERATION(i) POTHOS_PP_CAT(POTHOS_PP_RELATIVE_, i)(POTHOS_PP_ITERATION_)
#
# define POTHOS_PP_RELATIVE_0(m) POTHOS_PP_CAT(m, POTHOS_PP_ITERATION_DEPTH())
# define POTHOS_PP_RELATIVE_1(m) POTHOS_PP_CAT(m, POTHOS_PP_DEC(POTHOS_PP_ITERATION_DEPTH()))
# define POTHOS_PP_RELATIVE_2(m) POTHOS_PP_CAT(m, POTHOS_PP_DEC(POTHOS_PP_DEC(POTHOS_PP_ITERATION_DEPTH())))
# define POTHOS_PP_RELATIVE_3(m) POTHOS_PP_CAT(m, POTHOS_PP_DEC(POTHOS_PP_DEC(POTHOS_PP_DEC(POTHOS_PP_ITERATION_DEPTH()))))
# define POTHOS_PP_RELATIVE_4(m) POTHOS_PP_CAT(m, POTHOS_PP_DEC(POTHOS_PP_DEC(POTHOS_PP_DEC(POTHOS_PP_DEC(POTHOS_PP_ITERATION_DEPTH())))))
#
# /* POTHOS_PP_RELATIVE_START && POTHOS_PP_RELATIVE_FINISH */
#
# define POTHOS_PP_RELATIVE_START(i) POTHOS_PP_CAT(POTHOS_PP_RELATIVE_, i)(POTHOS_PP_ITERATION_START_)
# define POTHOS_PP_RELATIVE_FINISH(i) POTHOS_PP_CAT(POTHOS_PP_RELATIVE_, i)(POTHOS_PP_ITERATION_FINISH_)
#
# /* POTHOS_PP_RELATIVE_FLAGS */
#
# define POTHOS_PP_RELATIVE_FLAGS(i) (POTHOS_PP_CAT(POTHOS_PP_RELATIVE_, i)(POTHOS_PP_ITERATION_FLAGS_)())
#
# /* POTHOS_PP_ITERATE */
#
# define POTHOS_PP_ITERATE() POTHOS_PP_CAT(POTHOS_PP_ITERATE_, POTHOS_PP_INC(POTHOS_PP_ITERATION_DEPTH()))
#
# define POTHOS_PP_ITERATE_1 <Pothos/serialization/impl/preprocessor/iteration/detail/iter/forward1.hpp>
# define POTHOS_PP_ITERATE_2 <Pothos/serialization/impl/preprocessor/iteration/detail/iter/forward2.hpp>
# define POTHOS_PP_ITERATE_3 <Pothos/serialization/impl/preprocessor/iteration/detail/iter/forward3.hpp>
# define POTHOS_PP_ITERATE_4 <Pothos/serialization/impl/preprocessor/iteration/detail/iter/forward4.hpp>
# define POTHOS_PP_ITERATE_5 <Pothos/serialization/impl/preprocessor/iteration/detail/iter/forward5.hpp>
#
# endif
