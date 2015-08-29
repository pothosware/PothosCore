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
# ifndef POTHOS_PREPROCESSOR_SEQ_SEQ_HPP
# define POTHOS_PREPROCESSOR_SEQ_SEQ_HPP
#
# include <Pothos/serialization/impl/preprocessor/config/config.hpp>
# include <Pothos/serialization/impl/preprocessor/seq/elem.hpp>
#
# /* POTHOS_PP_SEQ_HEAD */
#
# define POTHOS_PP_SEQ_HEAD(seq) POTHOS_PP_SEQ_ELEM(0, seq)
#
# /* POTHOS_PP_SEQ_TAIL */
#
# if POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_MWCC()
#    define POTHOS_PP_SEQ_TAIL(seq) POTHOS_PP_SEQ_TAIL_1((seq))
#    define POTHOS_PP_SEQ_TAIL_1(par) POTHOS_PP_SEQ_TAIL_2 ## par
#    define POTHOS_PP_SEQ_TAIL_2(seq) POTHOS_PP_SEQ_TAIL_I ## seq
# elif POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_MSVC()
#    define POTHOS_PP_SEQ_TAIL(seq) POTHOS_PP_SEQ_TAIL_ID(POTHOS_PP_SEQ_TAIL_I seq)
#    define POTHOS_PP_SEQ_TAIL_ID(id) id
# elif POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_SEQ_TAIL(seq) POTHOS_PP_SEQ_TAIL_D(seq)
#    define POTHOS_PP_SEQ_TAIL_D(seq) POTHOS_PP_SEQ_TAIL_I seq
# else
#    define POTHOS_PP_SEQ_TAIL(seq) POTHOS_PP_SEQ_TAIL_I seq
# endif
#
# define POTHOS_PP_SEQ_TAIL_I(x)
#
# /* POTHOS_PP_SEQ_NIL */
#
# define POTHOS_PP_SEQ_NIL(x) (x)
#
# endif
