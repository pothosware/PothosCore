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
# ifndef POTHOS_PREPROCESSOR_SEQ_SUBSEQ_HPP
# define POTHOS_PREPROCESSOR_SEQ_SUBSEQ_HPP
#
# include <Pothos/serialization/impl/preprocessor/config/config.hpp>
# include <Pothos/serialization/impl/preprocessor/seq/first_n.hpp>
# include <Pothos/serialization/impl/preprocessor/seq/rest_n.hpp>
#
# /* POTHOS_PP_SEQ_SUBSEQ */
#
# if ~POTHOS_PP_CONFIG_FLAGS() & POTHOS_PP_CONFIG_EDG()
#    define POTHOS_PP_SEQ_SUBSEQ(seq, i, len) POTHOS_PP_SEQ_FIRST_N(len, POTHOS_PP_SEQ_REST_N(i, seq))
# else
#    define POTHOS_PP_SEQ_SUBSEQ(seq, i, len) POTHOS_PP_SEQ_SUBSEQ_I(seq, i, len)
#    define POTHOS_PP_SEQ_SUBSEQ_I(seq, i, len) POTHOS_PP_SEQ_FIRST_N(len, POTHOS_PP_SEQ_REST_N(i, seq))
# endif
#
# endif
