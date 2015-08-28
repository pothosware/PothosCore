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
# ifndef POTHOS_PREPROCESSOR_SLOT_SLOT_HPP
# define POTHOS_PREPROCESSOR_SLOT_SLOT_HPP
#
# include <Pothos/serialization/impl/preprocessor/cat.hpp>
# include <Pothos/serialization/impl/preprocessor/slot/detail/def.hpp>
#
# /* POTHOS_PP_ASSIGN_SLOT */
#
# define POTHOS_PP_ASSIGN_SLOT(i) POTHOS_PP_CAT(POTHOS_PP_ASSIGN_SLOT_, i)
#
# define POTHOS_PP_ASSIGN_SLOT_1 <Pothos/serialization/impl/preprocessor/slot/detail/slot1.hpp>
# define POTHOS_PP_ASSIGN_SLOT_2 <Pothos/serialization/impl/preprocessor/slot/detail/slot2.hpp>
# define POTHOS_PP_ASSIGN_SLOT_3 <Pothos/serialization/impl/preprocessor/slot/detail/slot3.hpp>
# define POTHOS_PP_ASSIGN_SLOT_4 <Pothos/serialization/impl/preprocessor/slot/detail/slot4.hpp>
# define POTHOS_PP_ASSIGN_SLOT_5 <Pothos/serialization/impl/preprocessor/slot/detail/slot5.hpp>
#
# /* POTHOS_PP_SLOT */
#
# define POTHOS_PP_SLOT(i) POTHOS_PP_CAT(POTHOS_PP_SLOT_, i)()
#
# endif
