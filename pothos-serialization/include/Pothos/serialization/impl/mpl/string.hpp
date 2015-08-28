
#ifndef POTHOS_MPL_STRING_HPP_INCLUDED
#define POTHOS_MPL_STRING_HPP_INCLUDED

// Copyright Eric Niebler 2009
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: string.hpp 49239 2009-04-01 09:10:26Z eric_niebler $
// $Date: 2009-04-01 02:10:26 -0700 (Wed, 1 Apr 2009) $
// $Revision: 49239 $
//
// Thanks to:
//   Dmitry Goncharov for porting this to the Sun compiler

#include <Pothos/serialization/impl/config.hpp>
#include <Pothos/serialization/impl/detail/workaround.hpp>
#include <Pothos/serialization/impl/detail/endian.hpp>
#include <Pothos/serialization/impl/mpl/limits/string.hpp>
#include <Pothos/serialization/impl/mpl/if.hpp>
#include <Pothos/serialization/impl/mpl/char.hpp>
#include <Pothos/serialization/impl/mpl/copy.hpp>
#include <Pothos/serialization/impl/mpl/size.hpp>
#include <Pothos/serialization/impl/mpl/empty.hpp>
#include <Pothos/serialization/impl/mpl/assert.hpp>
#include <Pothos/serialization/impl/mpl/size_t.hpp>
#include <Pothos/serialization/impl/mpl/begin_end.hpp>
#include <Pothos/serialization/impl/mpl/joint_view.hpp>
#include <Pothos/serialization/impl/mpl/insert_range.hpp>
#include <Pothos/serialization/impl/mpl/back_inserter.hpp>
#include <Pothos/serialization/impl/mpl/front_inserter.hpp>
#include <Pothos/serialization/impl/mpl/iterator_range.hpp>
#include <Pothos/serialization/impl/preprocessor/arithmetic/dec.hpp>
#include <Pothos/serialization/impl/preprocessor/arithmetic/add.hpp>
#include <Pothos/serialization/impl/preprocessor/arithmetic/div.hpp>
#include <Pothos/serialization/impl/preprocessor/punctuation/comma_if.hpp>
#include <Pothos/serialization/impl/preprocessor/repetition/repeat.hpp>
#include <Pothos/serialization/impl/preprocessor/repetition/enum_params.hpp>
#include <Pothos/serialization/impl/preprocessor/repetition/repeat_from_to.hpp>
#include <Pothos/serialization/impl/preprocessor/repetition/enum_shifted_params.hpp>
#include <Pothos/serialization/impl/preprocessor/repetition/enum_trailing_params.hpp>
#include <Pothos/serialization/impl/preprocessor/repetition/enum_params_with_a_default.hpp>

#include <iterator> // for bidirectional_iterator_tag
#include <climits>

namespace Pothos { namespace mpl
{
    #define POTHOS_MPL_STRING_MAX_PARAMS                                                             \
      POTHOS_PP_DIV(POTHOS_PP_ADD(POTHOS_MPL_LIMIT_STRING_SIZE, 3), 4)

    // Low-level bit-twiddling is done by macros. Any implementation-defined behavior of
    // multi-character literals should be localized to these macros.

    #define POTHOS_MPL_MULTICHAR_LENGTH(c)                                                           \
      (std::size_t)((c<CHAR_MIN) ? 4 : ((c>0xffffff)+(c>0xffff)+(c>0xff)+1))

    #if defined(POTHOS_LITTLE_ENDIAN) && defined(__SUNPRO_CC)

        #define POTHOS_MPL_MULTICHAR_AT(c,i)                                                         \
          (char)(0xff&((unsigned)(c)>>(8*(std::size_t)(i))))

        #define POTHOS_MPL_MULTICHAR_PUSH_BACK(c,i)                                                  \
          ((((unsigned char)(i))<<(POTHOS_MPL_MULTICHAR_LENGTH(c)*8))|(unsigned)(c))

        #define POTHOS_MPL_MULTICHAR_PUSH_FRONT(c,i)                                                 \
          (((unsigned)(c)<<8)|(unsigned char)(i))

        #define POTHOS_MPL_MULTICHAR_POP_BACK(c)                                                     \
          (((1<<((POTHOS_MPL_MULTICHAR_LENGTH(c)-1)*8))-1)&(unsigned)(c))

        #define POTHOS_MPL_MULTICHAR_POP_FRONT(c)                                                    \
          ((unsigned)(c)>>8)

    #else

        #define POTHOS_MPL_MULTICHAR_AT(c,i)                                                         \
          (char)(0xff&((unsigned)(c)>>(8*(POTHOS_MPL_MULTICHAR_LENGTH(c)-(std::size_t)(i)-1))))

        #define POTHOS_MPL_MULTICHAR_PUSH_BACK(c,i)                                                  \
          (((unsigned)(c)<<8)|(unsigned char)(i))

        #define POTHOS_MPL_MULTICHAR_PUSH_FRONT(c,i)                                                 \
          ((((unsigned char)(i))<<(POTHOS_MPL_MULTICHAR_LENGTH(c)*8))|(unsigned)(c))

        #define POTHOS_MPL_MULTICHAR_POP_BACK(c)                                                     \
          ((unsigned)(c)>>8)

        #define POTHOS_MPL_MULTICHAR_POP_FRONT(c)                                                    \
          (((1<<((POTHOS_MPL_MULTICHAR_LENGTH(c)-1)*8))-1)&(unsigned)(c))

    #endif

    struct string_tag;
    struct string_iterator_tag;

    template<POTHOS_PP_ENUM_PARAMS_WITH_A_DEFAULT(POTHOS_MPL_STRING_MAX_PARAMS, int C, 0)>
    struct string;

    template<typename Sequence, int I, int J>
    struct string_iterator;

    template<typename Sequence>
    struct sequence_tag;

    template<typename Tag>
    struct size_impl;

    template<>
    struct size_impl<mpl::string_tag>
    {
        template<typename Sequence>
        struct apply;

        #define M0(z, n, data)                                                                      \
        + POTHOS_MPL_MULTICHAR_LENGTH(POTHOS_PP_CAT(C,n))

        #define M1(z, n, data)                                                                      \
        template<POTHOS_PP_ENUM_PARAMS_Z(z, n, int C)>                                               \
        struct apply<mpl::string<POTHOS_PP_ENUM_PARAMS_Z(z, n, C)> >                                 \
          : mpl::size_t<(0 POTHOS_PP_REPEAT_ ## z(n, M0, ~))>                                        \
        {};

        POTHOS_PP_REPEAT_FROM_TO(1, POTHOS_PP_INC(POTHOS_MPL_STRING_MAX_PARAMS), M1, ~)
        #undef M0
        #undef M1
    };

    template<>
    struct size_impl<mpl::string_tag>::apply<mpl::string<> >
      : mpl::size_t<0>
    {};

    template<typename Tag>
    struct begin_impl;

    template<>
    struct begin_impl<mpl::string_tag>
    {
        template<typename Sequence>
        struct apply
        {
            typedef mpl::string_iterator<Sequence, 0, 0> type;
        };
    };

    template<typename Tag>
    struct end_impl;

    template<>
    struct end_impl<mpl::string_tag>
    {
        template<typename Sequence>
        struct apply;

        #define M0(z,n,data)                                                                        \
        template<POTHOS_PP_ENUM_PARAMS_Z(z, n, int C)>                                               \
        struct apply<mpl::string<POTHOS_PP_ENUM_PARAMS_Z(z, n, C)> >                                 \
        {                                                                                           \
            typedef mpl::string_iterator<mpl::string<POTHOS_PP_ENUM_PARAMS_Z(z, n, C)>, n, 0> type;  \
        };

        POTHOS_PP_REPEAT_FROM_TO(1, POTHOS_PP_INC(POTHOS_MPL_STRING_MAX_PARAMS), M0, ~)
        #undef M0
    };

    template<>
    struct end_impl<mpl::string_tag>::apply<mpl::string<> >
    {
        typedef mpl::string_iterator<mpl::string<>, 0, 0> type;
    };

    template<typename Tag>
    struct push_back_impl;

    template<>
    struct push_back_impl<mpl::string_tag>
    {
        template<typename Sequence, typename Value, bool B = (4==POTHOS_MPL_MULTICHAR_LENGTH(Sequence::back_))>
        struct apply
        {
            POTHOS_MPL_ASSERT_MSG(
                (POTHOS_MPL_LIMIT_STRING_SIZE != mpl::size<Sequence>::type::value)
              , PUSH_BACK_FAILED_MPL_STRING_IS_FULL
              , (Sequence)
            );
            // If the above assertion didn't fire, then the string is sparse.
            // Repack the string and retry the push_back
            typedef
                typename mpl::push_back<
                    typename mpl::copy<
                        Sequence
                      , mpl::back_inserter<mpl::string<> >
                    >::type
                  , Value
                >::type
            type;
        };

        template<typename Value>
        struct apply<mpl::string<>, Value, false>
        {
            typedef mpl::string<(char)Value::value> type;
        };

        #define M0(z,n,data)                                                                        \
        template<POTHOS_PP_ENUM_PARAMS_Z(z, n, int C), typename Value>                               \
        struct apply<mpl::string<POTHOS_PP_ENUM_PARAMS_Z(z, n, C)>, Value, false>                    \
        {                                                                                           \
            typedef                                                                                 \
                mpl::string<                                                                        \
                    POTHOS_PP_ENUM_PARAMS_Z(z, POTHOS_PP_DEC(n), C)                                   \
                    POTHOS_PP_COMMA_IF(POTHOS_PP_DEC(n))                                              \
                    ((unsigned)POTHOS_PP_CAT(C,POTHOS_PP_DEC(n))>0xffffff)                            \
                    ?POTHOS_PP_CAT(C,POTHOS_PP_DEC(n))                                                \
                    :POTHOS_MPL_MULTICHAR_PUSH_BACK(POTHOS_PP_CAT(C,POTHOS_PP_DEC(n)), Value::value)   \
                  , ((unsigned)POTHOS_PP_CAT(C,POTHOS_PP_DEC(n))>0xffffff)                            \
                    ?(char)Value::value                                                             \
                    :0                                                                              \
                >                                                                                   \
            type;                                                                                   \
        };

        POTHOS_PP_REPEAT_FROM_TO(1, POTHOS_MPL_STRING_MAX_PARAMS, M0, ~)
        #undef M0

        template<POTHOS_PP_ENUM_PARAMS(POTHOS_MPL_STRING_MAX_PARAMS, int C), typename Value>
        struct apply<mpl::string<POTHOS_PP_ENUM_PARAMS(POTHOS_MPL_STRING_MAX_PARAMS, C)>, Value, false>
        {
            typedef
                mpl::string<
                    POTHOS_PP_ENUM_PARAMS(POTHOS_PP_DEC(POTHOS_MPL_STRING_MAX_PARAMS), C)
                  , POTHOS_MPL_MULTICHAR_PUSH_BACK(POTHOS_PP_CAT(C,POTHOS_PP_DEC(POTHOS_MPL_STRING_MAX_PARAMS)), Value::value)
                >
            type;
        };
    };

    template<typename Tag>
    struct has_push_back_impl;

    template<>
    struct has_push_back_impl<mpl::string_tag>
    {
        template<typename Sequence>
        struct apply
          : mpl::true_
        {};
    };

    template<typename Tag>
    struct pop_back_impl;

    template<>
    struct pop_back_impl<mpl::string_tag>
    {
        template<typename Sequence>
        struct apply;

        #define M0(z,n,data)                                                                        \
        template<POTHOS_PP_ENUM_PARAMS_Z(z, n, int C)>                                               \
        struct apply<mpl::string<POTHOS_PP_ENUM_PARAMS_Z(z, n, C)> >                                 \
        {                                                                                           \
            POTHOS_MPL_ASSERT_MSG((C0 != 0), POP_BACK_FAILED_MPL_STRING_IS_EMPTY, (mpl::string<>));  \
            typedef                                                                                 \
                mpl::string<                                                                        \
                    POTHOS_PP_ENUM_PARAMS_Z(z, POTHOS_PP_DEC(n), C)                                   \
                    POTHOS_PP_COMMA_IF(POTHOS_PP_DEC(n))                                              \
                    POTHOS_MPL_MULTICHAR_POP_BACK(POTHOS_PP_CAT(C,POTHOS_PP_DEC(n)))                   \
                >                                                                                   \
            type;                                                                                   \
        };

        POTHOS_PP_REPEAT_FROM_TO(1, POTHOS_PP_INC(POTHOS_MPL_STRING_MAX_PARAMS), M0, ~)
        #undef M0
    };

    template<typename Tag>
    struct has_pop_back_impl;

    template<>
    struct has_pop_back_impl<mpl::string_tag>
    {
        template<typename Sequence>
        struct apply
          : mpl::true_
        {};
    };

    template<typename Tag>
    struct push_front_impl;

    template<>
    struct push_front_impl<mpl::string_tag>
    {
        template<typename Sequence, typename Value, bool B = (4==POTHOS_MPL_MULTICHAR_LENGTH(Sequence::front_))>
        struct apply
        {
            POTHOS_MPL_ASSERT_MSG(
                (POTHOS_MPL_LIMIT_STRING_SIZE != mpl::size<Sequence>::type::value)
              , PUSH_FRONT_FAILED_MPL_STRING_IS_FULL
              , (Sequence)
            );
            // If the above assertion didn't fire, then the string is sparse.
            // Repack the string and retry the push_front.
            typedef
                typename mpl::push_front<
                    typename mpl::reverse_copy<
                        Sequence
                      , mpl::front_inserter<string<> >
                    >::type
                  , Value
                >::type
            type;
        };

        #if !POTHOS_WORKAROUND(__SUNPRO_CC, POTHOS_TESTED_AT(0x590))
        template<typename Value>
        struct apply<mpl::string<>, Value, false>
        {
            typedef mpl::string<(char)Value::value> type;
        };
        #endif

        #define M0(z,n,data)                                                                        \
        template<POTHOS_PP_ENUM_PARAMS_Z(z, n, int C), typename Value>                               \
        struct apply<mpl::string<POTHOS_PP_ENUM_PARAMS_Z(z, n, C)>, Value, true>                     \
        {                                                                                           \
            typedef                                                                                 \
                mpl::string<                                                                        \
                    (char)Value::value                                                              \
                    POTHOS_PP_ENUM_TRAILING_PARAMS_Z(z, n, C)                                        \
                >                                                                                   \
            type;                                                                                   \
        };

        POTHOS_PP_REPEAT_FROM_TO(1, POTHOS_MPL_STRING_MAX_PARAMS, M0, ~)
        #undef M0

        template<POTHOS_PP_ENUM_PARAMS(POTHOS_MPL_STRING_MAX_PARAMS, int C), typename Value>
        struct apply<mpl::string<POTHOS_PP_ENUM_PARAMS(POTHOS_MPL_STRING_MAX_PARAMS, C)>, Value, false>
        {
            typedef
                mpl::string<
                    POTHOS_MPL_MULTICHAR_PUSH_FRONT(C0, Value::value)
                  , POTHOS_PP_ENUM_SHIFTED_PARAMS(POTHOS_MPL_STRING_MAX_PARAMS, C)
                >
            type0;

            #if POTHOS_WORKAROUND(__SUNPRO_CC, POTHOS_TESTED_AT(0x590))
            typedef
                typename mpl::if_<
                    mpl::empty<mpl::string<POTHOS_PP_ENUM_PARAMS(POTHOS_MPL_STRING_MAX_PARAMS, C)> >
                  , mpl::string<(char)Value::value>
                  , type0
                >::type
            type;
            #else
            typedef type0 type;
            #endif
        };
    };

    template<typename Tag>
    struct has_push_front_impl;

    template<>
    struct has_push_front_impl<mpl::string_tag>
    {
        template<typename Sequence>
        struct apply
          : mpl::true_
        {};
    };

    template<typename Tag>
    struct pop_front_impl;

    template<>
    struct pop_front_impl<mpl::string_tag>
    {
        template<typename Sequence, bool B = (1==POTHOS_MPL_MULTICHAR_LENGTH(Sequence::front_))>
        struct apply;

        #define M0(z,n,data)                                                                        \
        template<POTHOS_PP_ENUM_PARAMS_Z(z, n, int C)>                                               \
        struct apply<mpl::string<POTHOS_PP_ENUM_PARAMS_Z(z, n, C)>, true>                            \
        {                                                                                           \
            POTHOS_MPL_ASSERT_MSG((C0 != 0), POP_FRONT_FAILED_MPL_STRING_IS_EMPTY, (mpl::string<>)); \
            typedef                                                                                 \
                mpl::string<POTHOS_PP_ENUM_SHIFTED_PARAMS_Z(z, n, C)>                                \
            type;                                                                                   \
        };

        POTHOS_PP_REPEAT_FROM_TO(1, POTHOS_MPL_STRING_MAX_PARAMS, M0, ~)
        #undef M0

        template<POTHOS_PP_ENUM_PARAMS(POTHOS_MPL_STRING_MAX_PARAMS, int C)>
        struct apply<mpl::string<POTHOS_PP_ENUM_PARAMS(POTHOS_MPL_STRING_MAX_PARAMS, C)>, false>
        {
            typedef
                mpl::string<
                    POTHOS_MPL_MULTICHAR_POP_FRONT(C0)
                  , POTHOS_PP_ENUM_SHIFTED_PARAMS(POTHOS_MPL_STRING_MAX_PARAMS, C)
                >
            type;
        };
    };

    template<typename Tag>
    struct has_pop_front_impl;

    template<>
    struct has_pop_front_impl<mpl::string_tag>
    {
        template<typename Sequence>
        struct apply
          : mpl::true_
        {};
    };

    template<typename Tag>
    struct insert_range_impl;

    template<>
    struct insert_range_impl<mpl::string_tag>
    {
        template<typename Sequence, typename Pos, typename Range>
        struct apply
          : mpl::copy<
                mpl::joint_view<
                    mpl::iterator_range<
                        mpl::string_iterator<Sequence, 0, 0>
                      , Pos
                    >
                  , mpl::joint_view<
                        Range
                      , mpl::iterator_range<
                            Pos
                          , typename mpl::end<Sequence>::type
                        >
                    >
                >
              , mpl::back_inserter<mpl::string<> >
            >
        {};
    };

    template<typename Tag>
    struct insert_impl;

    template<>
    struct insert_impl<mpl::string_tag>
    {
        template<typename Sequence, typename Pos, typename Value>
        struct apply
          : mpl::insert_range<Sequence, Pos, mpl::string<(char)Value::value> >
        {};
    };

    template<typename Tag>
    struct erase_impl;

    template<>
    struct erase_impl<mpl::string_tag>
    {
        template<typename Sequence, typename First, typename Last>
        struct apply
          : mpl::copy<
                mpl::joint_view<
                    mpl::iterator_range<
                        mpl::string_iterator<Sequence, 0, 0>
                      , First
                    >
                  , mpl::iterator_range<
                        typename mpl::if_na<Last, typename mpl::next<First>::type>::type
                      , typename mpl::end<Sequence>::type
                    >
                >
              , mpl::back_inserter<mpl::string<> >
            >
        {};
    };

    template<typename Tag>
    struct clear_impl;

    template<>
    struct clear_impl<mpl::string_tag>
    {
        template<typename>
        struct apply
        {
            typedef mpl::string<> type;
        };
    };

    #define M0(z, n, data)                                                                            \
    template<POTHOS_PP_ENUM_PARAMS(POTHOS_MPL_STRING_MAX_PARAMS, int C), int J>                         \
    struct string_iterator<mpl::string<POTHOS_PP_ENUM_PARAMS(POTHOS_MPL_STRING_MAX_PARAMS, C)>, n, J>   \
    {                                                                                                 \
        enum { eomc_ = (POTHOS_MPL_MULTICHAR_LENGTH(POTHOS_PP_CAT(C, n)) == J + 1) };                   \
        typedef mpl::string<POTHOS_PP_ENUM_PARAMS(POTHOS_MPL_STRING_MAX_PARAMS, C)> string;             \
        typedef std::bidirectional_iterator_tag category;                                             \
        typedef                                                                                       \
            mpl::string_iterator<string, n + eomc_, eomc_ ? 0 : J + 1>                                \
        next;                                                                                         \
        typedef                                                                                       \
            mpl::string_iterator<string, n, J - 1>                                                    \
        prior;                                                                                        \
        typedef mpl::char_<POTHOS_MPL_MULTICHAR_AT(POTHOS_PP_CAT(C, n), J)> type;                       \
    };                                                                                                \
    template<POTHOS_PP_ENUM_PARAMS(POTHOS_MPL_STRING_MAX_PARAMS, int C)>                                \
    struct string_iterator<mpl::string<POTHOS_PP_ENUM_PARAMS(POTHOS_MPL_STRING_MAX_PARAMS, C)>, n, 0>   \
    {                                                                                                 \
        enum { eomc_ = (POTHOS_MPL_MULTICHAR_LENGTH(POTHOS_PP_CAT(C, n)) == 1) };                       \
        typedef mpl::string<POTHOS_PP_ENUM_PARAMS(POTHOS_MPL_STRING_MAX_PARAMS, C)> string;             \
        typedef std::bidirectional_iterator_tag category;                                             \
        typedef                                                                                       \
            mpl::string_iterator<string, n + eomc_, !eomc_>                                           \
        next;                                                                                         \
        typedef                                                                                       \
            mpl::string_iterator<                                                                     \
                string                                                                                \
              , n - 1                                                                                 \
              , POTHOS_MPL_MULTICHAR_LENGTH(POTHOS_PP_CAT(C, POTHOS_PP_DEC(n))) - 1                      \
            >                                                                                         \
        prior;                                                                                        \
        typedef mpl::char_<POTHOS_MPL_MULTICHAR_AT(POTHOS_PP_CAT(C, n), 0)> type;                       \
    };

    POTHOS_PP_REPEAT(POTHOS_MPL_STRING_MAX_PARAMS, M0, ~)
    #undef M0

    template<POTHOS_PP_ENUM_PARAMS(POTHOS_MPL_STRING_MAX_PARAMS, int C)>
    struct string
    {
        /// INTERNAL ONLY
        enum
        {
            front_  = C0
          , back_   = POTHOS_PP_CAT(C, POTHOS_PP_DEC(POTHOS_MPL_STRING_MAX_PARAMS))
        };

        typedef char        value_type;
        typedef string      type;
        typedef string_tag  tag;
    };

    namespace aux_
    {
        template<typename It, typename End>
        struct next_unless
          : mpl::next<It>
        {};

        template<typename End>
        struct next_unless<End, End>
        {
            typedef End type;
        };

        template<typename It, typename End>
        struct deref_unless
          : mpl::deref<It>
        {};

        template<typename End>
        struct deref_unless<End, End>
        {
            typedef mpl::char_<'\0'> type;
        };
    }

    template<typename Sequence>
    struct c_str
    {
        typedef typename mpl::end<Sequence>::type iend;
        typedef typename mpl::begin<Sequence>::type i0;
        #define M0(z, n, data)                                                                      \
        typedef                                                                                     \
            typename mpl::aux_::next_unless<POTHOS_PP_CAT(i, n), iend>::type                         \
        POTHOS_PP_CAT(i, POTHOS_PP_INC(n));
        POTHOS_PP_REPEAT(POTHOS_MPL_LIMIT_STRING_SIZE, M0, ~)
        #undef M0

        typedef c_str type;
        static typename Sequence::value_type const value[POTHOS_MPL_LIMIT_STRING_SIZE+1];
    };

    template<typename Sequence>
    typename Sequence::value_type const c_str<Sequence>::value[POTHOS_MPL_LIMIT_STRING_SIZE+1] =
    {
        #define M0(z, n, data)                                                                      \
        mpl::aux_::deref_unless<POTHOS_PP_CAT(i, n), iend>::type::value,
        POTHOS_PP_REPEAT(POTHOS_MPL_LIMIT_STRING_SIZE, M0, ~)
        #undef M0
        '\0'
    };

}} // namespace boost

#endif // BOOST_MPL_STRING_HPP_INCLUDED
