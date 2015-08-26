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
# ifndef POTHOS_PREPROCESSOR_SEQ_FOR_EACH_I_HPP
# define POTHOS_PREPROCESSOR_SEQ_FOR_EACH_I_HPP
#
# include <Pothos/serialization/impl/preprocessor/arithmetic/dec.hpp>
# include <Pothos/serialization/impl/preprocessor/arithmetic/inc.hpp>
# include <Pothos/serialization/impl/preprocessor/config/config.hpp>
# include <Pothos/serialization/impl/preprocessor/repetition/for.hpp>
# include <Pothos/serialization/impl/preprocessor/seq/seq.hpp>
# include <Pothos/serialization/impl/preprocessor/seq/size.hpp>
# include <Pothos/serialization/impl/preprocessor/tuple/elem.hpp>
# include <Pothos/serialization/impl/preprocessor/tuple/rem.hpp>
#
# /* POTHOS_PP_SEQ_FOR_EACH_I */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_SEQ_FOR_EACH_I(macro, data, seq) POTHOS_PP_FOR((macro, data, seq (nil), 0), POTHOS_PP_SEQ_FOR_EACH_I_P, POTHOS_PP_SEQ_FOR_EACH_I_O, POTHOS_PP_SEQ_FOR_EACH_I_M)
# else
#    define POTHOS_PP_SEQ_FOR_EACH_I(macro, data, seq) POTHOS_PP_SEQ_FOR_EACH_I_I(macro, data, seq)
#    define POTHOS_PP_SEQ_FOR_EACH_I_I(macro, data, seq) POTHOS_PP_FOR((macro, data, seq (nil), 0), POTHOS_PP_SEQ_FOR_EACH_I_P, POTHOS_PP_SEQ_FOR_EACH_I_O, POTHOS_PP_SEQ_FOR_EACH_I_M)
# endif
#
# define POTHOS_PP_SEQ_FOR_EACH_I_P(r, x) POTHOS_PP_DEC(POTHOS_PP_SEQ_SIZE(POTHOS_PP_TUPLE_ELEM(4, 2, x)))
#
# if POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_STRICT()
#    define POTHOS_PP_SEQ_FOR_EACH_I_O(r, x) POTHOS_PP_SEQ_FOR_EACH_I_O_I x
# else
#    define POTHOS_PP_SEQ_FOR_EACH_I_O(r, x) POTHOS_PP_SEQ_FOR_EACH_I_O_I(POTHOS_PP_TUPLE_ELEM(4, 0, x), POTHOS_PP_TUPLE_ELEM(4, 1, x), POTHOS_PP_TUPLE_ELEM(4, 2, x), POTHOS_PP_TUPLE_ELEM(4, 3, x))
# endif
#
# define POTHOS_PP_SEQ_FOR_EACH_I_O_I(macro, data, seq, i) (macro, data, POTHOS_PP_SEQ_TAIL(seq), POTHOS_PP_INC(i))
#
# if POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_STRICT()
#    define POTHOS_PP_SEQ_FOR_EACH_I_M(r, x) POTHOS_PP_SEQ_FOR_EACH_I_M_IM(r, POTHOS_PP_TUPLE_REM_4 x)
#    define POTHOS_PP_SEQ_FOR_EACH_I_M_IM(r, im) POTHOS_PP_SEQ_FOR_EACH_I_M_I(r, im)
# else
#    define POTHOS_PP_SEQ_FOR_EACH_I_M(r, x) POTHOS_PP_SEQ_FOR_EACH_I_M_I(r, POTHOS_PP_TUPLE_ELEM(4, 0, x), POTHOS_PP_TUPLE_ELEM(4, 1, x), POTHOS_PP_TUPLE_ELEM(4, 2, x), POTHOS_PP_TUPLE_ELEM(4, 3, x))
# endif
#
# define POTHOS_PP_SEQ_FOR_EACH_I_M_I(r, macro, data, seq, i) macro(r, data, i, POTHOS_PP_SEQ_HEAD(seq))
#
# /* POTHOS_PP_SEQ_FOR_EACH_I_R */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_SEQ_FOR_EACH_I_R(r, macro, data, seq) POTHOS_PP_FOR_ ## r((macro, data, seq (nil), 0), POTHOS_PP_SEQ_FOR_EACH_I_P, POTHOS_PP_SEQ_FOR_EACH_I_O, POTHOS_PP_SEQ_FOR_EACH_I_M)
# else
#    define POTHOS_PP_SEQ_FOR_EACH_I_R(r, macro, data, seq) POTHOS_PP_SEQ_FOR_EACH_I_R_I(r, macro, data, seq)
#    define POTHOS_PP_SEQ_FOR_EACH_I_R_I(r, macro, data, seq) POTHOS_PP_FOR_ ## r((macro, data, seq (nil), 0), POTHOS_PP_SEQ_FOR_EACH_I_P, POTHOS_PP_SEQ_FOR_EACH_I_O, POTHOS_PP_SEQ_FOR_EACH_I_M)
# endif
#
# endif
