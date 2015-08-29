
#ifndef POTHOS_MPL_HAS_XXX_HPP_INCLUDED
#define POTHOS_MPL_HAS_XXX_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2002-2006
// Copyright David Abrahams 2002-2003
// Copyright Daniel Walker 2007
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: has_xxx.hpp 64146 2010-07-19 00:46:31Z djwalker $
// $Date: 2010-07-18 17:46:31 -0700 (Sun, 18 Jul 2010) $
// $Revision: 64146 $

#include <Pothos/serialization/impl/mpl/bool.hpp>
#include <Pothos/serialization/impl/mpl/aux_/na_spec.hpp>
#include <Pothos/serialization/impl/mpl/aux_/type_wrapper.hpp>
#include <Pothos/serialization/impl/mpl/aux_/yes_no.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/gcc.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/has_xxx.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/msvc_typename.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/msvc.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/static_constant.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/workaround.hpp>

#include <Pothos/serialization/impl/preprocessor/array/elem.hpp>
#include <Pothos/serialization/impl/preprocessor/cat.hpp>
#include <Pothos/serialization/impl/preprocessor/control/if.hpp>
#include <Pothos/serialization/impl/preprocessor/repetition/enum_params.hpp>
#include <Pothos/serialization/impl/preprocessor/repetition/enum_trailing_params.hpp>

#if POTHOS_WORKAROUND( __BORLANDC__, POTHOS_TESTED_AT(0x590) )
# include <Pothos/serialization/impl/type_traits/is_class.hpp>
#endif

#if !defined(POTHOS_MPL_CFG_NO_HAS_XXX)

#   if POTHOS_WORKAROUND(POTHOS_MSVC, <= 1300)

// agurt, 11/sep/02: MSVC-specific version (< 7.1), based on a USENET 
// newsgroup's posting by John Madsen (comp.lang.c++.moderated, 
// 1999-11-12 19:17:06 GMT); the code is _not_ standard-conforming, but 
// it works way more reliably than the SFINAE-based implementation

// Modified dwa 8/Oct/02 to handle reference types.

#   include <Pothos/serialization/impl/mpl/if.hpp>
#   include <Pothos/serialization/impl/mpl/bool.hpp>

namespace Pothos { namespace mpl { namespace aux {

struct has_xxx_tag;

#if POTHOS_WORKAROUND(POTHOS_MSVC, == 1300)
template< typename U > struct msvc_incomplete_array
{
    typedef char (&type)[sizeof(U) + 1];
};
#endif

template< typename T >
struct msvc_is_incomplete
{
    // MSVC is capable of some kinds of SFINAE.  If U is an incomplete
    // type, it won't pick the second overload
    static char tester(...);

#if POTHOS_WORKAROUND(POTHOS_MSVC, == 1300)
    template< typename U >
    static typename msvc_incomplete_array<U>::type tester(type_wrapper<U>);
#else
    template< typename U >
    static char (& tester(type_wrapper<U>) )[sizeof(U)+1];
#endif 
    
    POTHOS_STATIC_CONSTANT(bool, value = 
          sizeof(tester(type_wrapper<T>())) == 1
        );
};

template<>
struct msvc_is_incomplete<int>
{
    POTHOS_STATIC_CONSTANT(bool, value = false);
};

}}}

#   define POTHOS_MPL_HAS_XXX_TRAIT_NAMED_DEF_(trait, name, default_) \
template< typename T, typename name = ::Pothos::mpl::aux::has_xxx_tag > \
struct POTHOS_PP_CAT(trait,_impl) : T \
{ \
    static Pothos::mpl::aux::no_tag \
    test(void(*)(::Pothos::mpl::aux::has_xxx_tag)); \
    \
    static Pothos::mpl::aux::yes_tag test(...); \
    \
    POTHOS_STATIC_CONSTANT(bool, value = \
          sizeof(test(static_cast<void(*)(name)>(0))) \
            != sizeof(Pothos::mpl::aux::no_tag) \
        ); \
    typedef Pothos::mpl::bool_<value> type; \
}; \
\
template< typename T, typename fallback_ = Pothos::mpl::bool_<default_> > \
struct trait \
    : Pothos::mpl::if_c< \
          Pothos::mpl::aux::msvc_is_incomplete<T>::value \
        , Pothos::mpl::bool_<false> \
        , POTHOS_PP_CAT(trait,_impl)<T> \
        >::type \
{ \
}; \
\
POTHOS_MPL_AUX_HAS_XXX_TRAIT_SPEC(trait, void) \
POTHOS_MPL_AUX_HAS_XXX_TRAIT_SPEC(trait, bool) \
POTHOS_MPL_AUX_HAS_XXX_TRAIT_SPEC(trait, char) \
POTHOS_MPL_AUX_HAS_XXX_TRAIT_SPEC(trait, signed char) \
POTHOS_MPL_AUX_HAS_XXX_TRAIT_SPEC(trait, unsigned char) \
POTHOS_MPL_AUX_HAS_XXX_TRAIT_SPEC(trait, signed short) \
POTHOS_MPL_AUX_HAS_XXX_TRAIT_SPEC(trait, unsigned short) \
POTHOS_MPL_AUX_HAS_XXX_TRAIT_SPEC(trait, signed int) \
POTHOS_MPL_AUX_HAS_XXX_TRAIT_SPEC(trait, unsigned int) \
POTHOS_MPL_AUX_HAS_XXX_TRAIT_SPEC(trait, signed long) \
POTHOS_MPL_AUX_HAS_XXX_TRAIT_SPEC(trait, unsigned long) \
POTHOS_MPL_AUX_HAS_XXX_TRAIT_SPEC(trait, float) \
POTHOS_MPL_AUX_HAS_XXX_TRAIT_SPEC(trait, double) \
POTHOS_MPL_AUX_HAS_XXX_TRAIT_SPEC(trait, long double) \
/**/

#   define POTHOS_MPL_AUX_HAS_XXX_TRAIT_SPEC(trait, T) \
template<> struct trait<T> \
{ \
    POTHOS_STATIC_CONSTANT(bool, value = false); \
    typedef Pothos::mpl::bool_<false> type; \
}; \
/**/

#if !defined(POTHOS_NO_INTRINSIC_WCHAR_T)
#   define POTHOS_MPL_HAS_XXX_TRAIT_NAMED_DEF(trait, name, unused) \
    POTHOS_MPL_HAS_XXX_TRAIT_NAMED_DEF_(trait, name, unused) \
    POTHOS_MPL_AUX_HAS_XXX_TRAIT_SPEC(trait, wchar_t) \
/**/
#else
#   define POTHOS_MPL_HAS_XXX_TRAIT_NAMED_DEF(trait, name, unused) \
    POTHOS_MPL_HAS_XXX_TRAIT_NAMED_DEF_(trait, name, unused) \
/**/
#endif


// SFINAE-based implementations below are derived from a USENET newsgroup's 
// posting by Rani Sharoni (comp.lang.c++.moderated, 2002-03-17 07:45:09 PST)

#   elif POTHOS_WORKAROUND(POTHOS_MSVC, POTHOS_TESTED_AT(1400)) \
      || POTHOS_WORKAROUND(__IBMCPP__, <= 700)

// MSVC 7.1+ & VACPP

// agurt, 15/jun/05: replace overload-based SFINAE implementation with SFINAE
// applied to partial specialization to fix some apparently random failures 
// (thanks to Daniel Wallin for researching this!)

#   define POTHOS_MPL_HAS_XXX_TRAIT_NAMED_DEF(trait, name, default_) \
template< typename T > \
struct POTHOS_PP_CAT(trait, _msvc_sfinae_helper) \
{ \
    typedef void type; \
};\
\
template< typename T, typename U = void > \
struct POTHOS_PP_CAT(trait,_impl_) \
{ \
    POTHOS_STATIC_CONSTANT(bool, value = false); \
    typedef Pothos::mpl::bool_<value> type; \
}; \
\
template< typename T > \
struct POTHOS_PP_CAT(trait,_impl_)< \
      T \
    , typename POTHOS_PP_CAT(trait, _msvc_sfinae_helper)< typename T::name >::type \
    > \
{ \
    POTHOS_STATIC_CONSTANT(bool, value = true); \
    typedef Pothos::mpl::bool_<value> type; \
}; \
\
template< typename T, typename fallback_ = Pothos::mpl::bool_<default_> > \
struct trait \
    : POTHOS_PP_CAT(trait,_impl_)<T> \
{ \
}; \
/**/

#   elif POTHOS_WORKAROUND( __BORLANDC__, POTHOS_TESTED_AT(0x590) )

#   define POTHOS_MPL_HAS_XXX_TRAIT_NAMED_BCB_DEF(trait, trait_tester, name, default_) \
template< typename T, bool IS_CLASS > \
struct trait_tester \
{ \
    POTHOS_STATIC_CONSTANT( bool,  value = false ); \
}; \
template< typename T > \
struct trait_tester< T, true > \
{ \
    struct trait_tester_impl \
    { \
        template < class U > \
        static int  resolve( Pothos::mpl::aux::type_wrapper<U> const volatile * \
                           , Pothos::mpl::aux::type_wrapper<typename U::name >* = 0 ); \
        static char resolve( ... ); \
    }; \
    typedef Pothos::mpl::aux::type_wrapper<T> t_; \
    POTHOS_STATIC_CONSTANT( bool, value = ( sizeof( trait_tester_impl::resolve( static_cast< t_ * >(0) ) ) == sizeof(int) ) ); \
}; \
template< typename T, typename fallback_ = Pothos::mpl::bool_<default_> > \
struct trait           \
{                      \
    POTHOS_STATIC_CONSTANT( bool, value = (trait_tester< T, Pothos::is_class< T >::value >::value) );     \
    typedef Pothos::mpl::bool_< trait< T, fallback_ >::value > type; \
};

#   define POTHOS_MPL_HAS_XXX_TRAIT_NAMED_DEF(trait, name, default_) \
    POTHOS_MPL_HAS_XXX_TRAIT_NAMED_BCB_DEF( trait \
                                         , POTHOS_PP_CAT(trait,_tester)      \
                                         , name       \
                                         , default_ ) \
/**/

#   else // other SFINAE-capable compilers

#   define POTHOS_MPL_HAS_XXX_TRAIT_NAMED_DEF(trait, name, default_) \
template< typename T, typename fallback_ = Pothos::mpl::bool_<default_> > \
struct trait \
{ \
    struct gcc_3_2_wknd \
    { \
        template< typename U > \
        static Pothos::mpl::aux::yes_tag test( \
              Pothos::mpl::aux::type_wrapper<U> const volatile* \
            , Pothos::mpl::aux::type_wrapper<POTHOS_MSVC_TYPENAME U::name>* = 0 \
            ); \
    \
        static Pothos::mpl::aux::no_tag test(...); \
    }; \
    \
    typedef Pothos::mpl::aux::type_wrapper<T> t_; \
    POTHOS_STATIC_CONSTANT(bool, value = \
          sizeof(gcc_3_2_wknd::test(static_cast<t_*>(0))) \
            == sizeof(Pothos::mpl::aux::yes_tag) \
        ); \
    typedef Pothos::mpl::bool_<value> type; \
}; \
/**/

#   endif // BOOST_WORKAROUND(BOOST_MSVC, <= 1300)


#else // BOOST_MPL_CFG_NO_HAS_XXX

// placeholder implementation

#   define POTHOS_MPL_HAS_XXX_TRAIT_NAMED_DEF(trait, name, default_) \
template< typename T, typename fallback_ = Pothos::mpl::bool_<default_> > \
struct trait \
{ \
    POTHOS_STATIC_CONSTANT(bool, value = fallback_::value); \
    typedef fallback_ type; \
}; \
/**/

#endif

#define POTHOS_MPL_HAS_XXX_TRAIT_DEF(name) \
    POTHOS_MPL_HAS_XXX_TRAIT_NAMED_DEF(POTHOS_PP_CAT(has_,name), name, false) \
/**/


#if !defined(POTHOS_MPL_CFG_NO_HAS_XXX_TEMPLATE)

// Create a boolean Metafunction to detect a nested template
// member. This implementation is based on a USENET newsgroup's
// posting by Aleksey Gurtovoy (comp.lang.c++.moderated, 2002-03-19),
// Rani Sharoni's USENET posting cited above, the non-template has_xxx
// implementations above, and discussion on the Boost mailing list.

#   if !defined(POTHOS_MPL_HAS_XXX_NO_WRAPPED_TYPES)
#     if POTHOS_WORKAROUND(POTHOS_MSVC, <= 1400)
#       define POTHOS_MPL_HAS_XXX_NO_WRAPPED_TYPES 1
#     endif
#   endif

#   if !defined(POTHOS_MPL_HAS_XXX_NO_EXPLICIT_TEST_FUNCTION)
#     if (defined(POTHOS_NO_EXPLICIT_FUNCTION_TEMPLATE_ARGUMENTS))
#       define POTHOS_MPL_HAS_XXX_NO_EXPLICIT_TEST_FUNCTION 1
#     endif
#   endif

#   if !defined(POTHOS_MPL_HAS_XXX_NEEDS_TEMPLATE_SFINAE)
#     if POTHOS_WORKAROUND(POTHOS_MSVC, <= 1400)
#       define POTHOS_MPL_HAS_XXX_NEEDS_TEMPLATE_SFINAE 1
#     endif
#   endif

// NOTE: Many internal implementation macros take a Boost.Preprocessor
// array argument called args which is of the following form.
//           ( 4, ( trait, name, max_arity, default_ ) )

#   define POTHOS_MPL_HAS_MEMBER_INTROSPECTION_NAME(args) \
      POTHOS_PP_CAT(POTHOS_PP_ARRAY_ELEM(0, args) , _introspect) \
    /**/

#   define POTHOS_MPL_HAS_MEMBER_INTROSPECTION_SUBSTITUTE_NAME(args, n) \
      POTHOS_PP_CAT(POTHOS_PP_CAT(POTHOS_PP_ARRAY_ELEM(0, args) , _substitute), n) \
    /**/

#   define POTHOS_MPL_HAS_MEMBER_INTROSPECTION_TEST_NAME(args) \
      POTHOS_PP_CAT(POTHOS_PP_ARRAY_ELEM(0, args) , _test) \
    /**/

// Thanks to Guillaume Melquiond for pointing out the need for the
// "substitute" template as an argument to the overloaded test
// functions to get SFINAE to work for member templates with the
// correct name but different number of arguments.
#   define POTHOS_MPL_HAS_MEMBER_MULTI_SUBSTITUTE(z, n, args) \
      template< \
          template< POTHOS_PP_ENUM_PARAMS(POTHOS_PP_INC(n), typename V) > class V \
       > \
      struct POTHOS_MPL_HAS_MEMBER_INTROSPECTION_SUBSTITUTE_NAME(args, n) { \
      }; \
    /**/

#   define POTHOS_MPL_HAS_MEMBER_SUBSTITUTE(args, substitute_macro) \
      POTHOS_PP_REPEAT( \
          POTHOS_PP_ARRAY_ELEM(2, args) \
        , POTHOS_MPL_HAS_MEMBER_MULTI_SUBSTITUTE \
        , args \
      ) \
    /**/

#   if !POTHOS_MPL_HAS_XXX_NO_EXPLICIT_TEST_FUNCTION
#     define POTHOS_MPL_HAS_MEMBER_REJECT(args, member_macro) \
        template< typename V > \
        static Pothos::mpl::aux::no_tag \
        POTHOS_MPL_HAS_MEMBER_INTROSPECTION_TEST_NAME(args)(...); \
      /**/
#   else
#     define POTHOS_MPL_HAS_MEMBER_REJECT(args, member_macro) \
        static Pothos::mpl::aux::no_tag \
        POTHOS_MPL_HAS_MEMBER_INTROSPECTION_TEST_NAME(args)(...); \
      /**/
#   endif

#   if !POTHOS_MPL_HAS_XXX_NO_WRAPPED_TYPES
#     define POTHOS_MPL_HAS_MEMBER_MULTI_ACCEPT(z, n, args) \
        template< typename V > \
        static Pothos::mpl::aux::yes_tag \
        POTHOS_MPL_HAS_MEMBER_INTROSPECTION_TEST_NAME(args)( \
            Pothos::mpl::aux::type_wrapper< V > const volatile* \
          , POTHOS_MPL_HAS_MEMBER_INTROSPECTION_SUBSTITUTE_NAME(args, n) < \
                V::template POTHOS_PP_ARRAY_ELEM(1, args) \
            >* = 0 \
        ); \
      /**/
#     define POTHOS_MPL_HAS_MEMBER_ACCEPT(args, member_macro) \
        POTHOS_PP_REPEAT( \
            POTHOS_PP_ARRAY_ELEM(2, args) \
          , POTHOS_MPL_HAS_MEMBER_MULTI_ACCEPT \
          , args \
        ) \
      /**/
#   else
#     define POTHOS_MPL_HAS_MEMBER_ACCEPT(args, member_macro) \
        template< typename V > \
        static Pothos::mpl::aux::yes_tag \
        POTHOS_MPL_HAS_MEMBER_INTROSPECTION_TEST_NAME(args)( \
            V const volatile* \
          , member_macro(args, V, T)* = 0 \
        ); \
      /**/
#   endif

#   if !POTHOS_MPL_HAS_XXX_NO_EXPLICIT_TEST_FUNCTION
#     define POTHOS_MPL_HAS_MEMBER_TEST(args) \
          sizeof(POTHOS_MPL_HAS_MEMBER_INTROSPECTION_TEST_NAME(args)< U >(0)) \
              == sizeof(Pothos::mpl::aux::yes_tag) \
      /**/
#   else
#     if !POTHOS_MPL_HAS_XXX_NO_WRAPPED_TYPES
#       define POTHOS_MPL_HAS_MEMBER_TEST(args) \
          sizeof( \
              POTHOS_MPL_HAS_MEMBER_INTROSPECTION_TEST_NAME(args)( \
                  static_cast< Pothos::mpl::aux::type_wrapper< U >* >(0) \
              ) \
          ) == sizeof(Pothos::mpl::aux::yes_tag) \
        /**/
#     else
#       define POTHOS_MPL_HAS_MEMBER_TEST(args) \
          sizeof( \
              POTHOS_MPL_HAS_MEMBER_INTROSPECTION_TEST_NAME(args)( \
                  static_cast< U* >(0) \
              ) \
          ) == sizeof(Pothos::mpl::aux::yes_tag) \
        /**/
#     endif
#   endif

#   define POTHOS_MPL_HAS_MEMBER_INTROSPECT( \
               args, substitute_macro, member_macro \
           ) \
      template< typename U > \
      struct POTHOS_MPL_HAS_MEMBER_INTROSPECTION_NAME(args) { \
          POTHOS_MPL_HAS_MEMBER_SUBSTITUTE(args, substitute_macro) \
          POTHOS_MPL_HAS_MEMBER_REJECT(args, member_macro) \
          POTHOS_MPL_HAS_MEMBER_ACCEPT(args, member_macro) \
          POTHOS_STATIC_CONSTANT( \
              bool, value = POTHOS_MPL_HAS_MEMBER_TEST(args) \
          ); \
          typedef Pothos::mpl::bool_< value > type; \
      }; \
    /**/

#   define POTHOS_MPL_HAS_MEMBER_IMPLEMENTATION( \
               args, introspect_macro, substitute_macro, member_macro \
           ) \
      template< \
          typename T \
        , typename fallback_ \
              = Pothos::mpl::bool_< POTHOS_PP_ARRAY_ELEM(3, args) > \
      > \
      class POTHOS_PP_ARRAY_ELEM(0, args) { \
          introspect_macro(args, substitute_macro, member_macro) \
      public: \
          static const bool value \
              = POTHOS_MPL_HAS_MEMBER_INTROSPECTION_NAME(args)< T >::value; \
          typedef typename POTHOS_MPL_HAS_MEMBER_INTROSPECTION_NAME(args)< \
              T \
          >::type type; \
      }; \
    /**/

// BOOST_MPL_HAS_MEMBER_WITH_FUNCTION_SFINAE expands to the full
// implementation of the function-based metafunction. Compile with -E
// to see the preprocessor output for this macro.
#   define POTHOS_MPL_HAS_MEMBER_WITH_FUNCTION_SFINAE( \
               args, substitute_macro, member_macro \
           ) \
      POTHOS_MPL_HAS_MEMBER_IMPLEMENTATION( \
          args \
        , POTHOS_MPL_HAS_MEMBER_INTROSPECT \
        , substitute_macro \
        , member_macro \
      ) \
    /**/

#   if POTHOS_MPL_HAS_XXX_NEEDS_TEMPLATE_SFINAE

#     if !defined(POTHOS_MPL_HAS_XXX_NEEDS_NAMESPACE_LEVEL_SUBSTITUTE)
#       if POTHOS_WORKAROUND(POTHOS_MSVC, <= 1400)
#         define POTHOS_MPL_HAS_XXX_NEEDS_NAMESPACE_LEVEL_SUBSTITUTE 1
#       endif
#     endif

#     if !POTHOS_MPL_HAS_XXX_NEEDS_NAMESPACE_LEVEL_SUBSTITUTE
#       define POTHOS_MPL_HAS_MEMBER_INTROSPECTION_SUBSTITUTE_NAME_WITH_TEMPLATE_SFINAE( \
                   args, n \
               ) \
          POTHOS_MPL_HAS_MEMBER_INTROSPECTION_SUBSTITUTE_NAME(args, n) \
        /**/
#     else
#       define POTHOS_MPL_HAS_MEMBER_INTROSPECTION_SUBSTITUTE_NAME_WITH_TEMPLATE_SFINAE( \
                   args, n \
               ) \
          POTHOS_PP_CAT( \
              Pothos_mpl_has_xxx_ \
            , POTHOS_MPL_HAS_MEMBER_INTROSPECTION_SUBSTITUTE_NAME(args, n) \
          ) \
        /**/
#     endif

#     define POTHOS_MPL_HAS_MEMBER_INTROSPECTION_SUBSTITUTE_TAG_NAME( \
                 args \
             ) \
        POTHOS_PP_CAT( \
            POTHOS_MPL_HAS_MEMBER_INTROSPECTION_SUBSTITUTE_NAME_WITH_TEMPLATE_SFINAE( \
                args, 0 \
            ) \
          , _tag \
        ) \
      /**/

#     define POTHOS_MPL_HAS_MEMBER_MULTI_SUBSTITUTE_WITH_TEMPLATE_SFINAE( \
                 z, n, args \
             ) \
        template< \
             template< POTHOS_PP_ENUM_PARAMS(POTHOS_PP_INC(n), typename U) > class U \
        > \
        struct POTHOS_MPL_HAS_MEMBER_INTROSPECTION_SUBSTITUTE_NAME_WITH_TEMPLATE_SFINAE( \
                args, n \
               ) { \
            typedef \
                POTHOS_MPL_HAS_MEMBER_INTROSPECTION_SUBSTITUTE_TAG_NAME(args) \
                type; \
        }; \
      /**/

#     define POTHOS_MPL_HAS_MEMBER_SUBSTITUTE_WITH_TEMPLATE_SFINAE( \
                 args, substitute_macro \
             ) \
        typedef void \
            POTHOS_MPL_HAS_MEMBER_INTROSPECTION_SUBSTITUTE_TAG_NAME(args); \
        POTHOS_PP_REPEAT( \
            POTHOS_PP_ARRAY_ELEM(2, args) \
          , POTHOS_MPL_HAS_MEMBER_MULTI_SUBSTITUTE_WITH_TEMPLATE_SFINAE \
          , args \
        ) \
      /**/

#     define POTHOS_MPL_HAS_MEMBER_REJECT_WITH_TEMPLATE_SFINAE( \
                 args, member_macro \
             ) \
        template< \
            typename U \
          , typename V \
                = POTHOS_MPL_HAS_MEMBER_INTROSPECTION_SUBSTITUTE_TAG_NAME(args) \
        > \
        struct POTHOS_MPL_HAS_MEMBER_INTROSPECTION_TEST_NAME(args) { \
            POTHOS_STATIC_CONSTANT(bool, value = false); \
            typedef Pothos::mpl::bool_< value > type; \
        }; \
      /**/

#     define POTHOS_MPL_HAS_MEMBER_MULTI_ACCEPT_WITH_TEMPLATE_SFINAE( \
                 z, n, args \
             ) \
        template< typename U > \
        struct POTHOS_MPL_HAS_MEMBER_INTROSPECTION_TEST_NAME(args)< \
            U \
          , typename \
                POTHOS_MPL_HAS_MEMBER_INTROSPECTION_SUBSTITUTE_NAME_WITH_TEMPLATE_SFINAE( \
                    args, n \
                )< \
                    POTHOS_MSVC_TYPENAME U::POTHOS_PP_ARRAY_ELEM(1, args)< > \
                >::type \
        > { \
            POTHOS_STATIC_CONSTANT(bool, value = true); \
            typedef Pothos::mpl::bool_< value > type; \
        }; \
      /**/

#     define POTHOS_MPL_HAS_MEMBER_ACCEPT_WITH_TEMPLATE_SFINAE( \
                 args, member_macro \
             ) \
        POTHOS_PP_REPEAT( \
            POTHOS_PP_ARRAY_ELEM(2, args) \
          , POTHOS_MPL_HAS_MEMBER_MULTI_ACCEPT_WITH_TEMPLATE_SFINAE \
          , args \
        ) \
      /**/

#     define POTHOS_MPL_HAS_MEMBER_INTROSPECT_WITH_TEMPLATE_SFINAE( \
                 args, substitute_macro, member_macro \
             ) \
        POTHOS_MPL_HAS_MEMBER_REJECT_WITH_TEMPLATE_SFINAE(args, member_macro) \
        POTHOS_MPL_HAS_MEMBER_ACCEPT_WITH_TEMPLATE_SFINAE(args, member_macro) \
        template< typename U > \
        struct POTHOS_MPL_HAS_MEMBER_INTROSPECTION_NAME(args) \
            : POTHOS_MPL_HAS_MEMBER_INTROSPECTION_TEST_NAME(args)< U > { \
        }; \
      /**/
 
// BOOST_MPL_HAS_MEMBER_WITH_TEMPLATE_SFINAE expands to the full
// implementation of the template-based metafunction. Compile with -E
// to see the preprocessor output for this macro.
//
// Note that if BOOST_MPL_HAS_XXX_NEEDS_NAMESPACE_LEVEL_SUBSTITUTE is
// defined BOOST_MPL_HAS_MEMBER_SUBSTITUTE_WITH_TEMPLATE_SFINAE needs
// to be expanded at namespace level before
// BOOST_MPL_HAS_MEMBER_WITH_TEMPLATE_SFINAE can be used.
#     define POTHOS_MPL_HAS_MEMBER_WITH_TEMPLATE_SFINAE( \
                 args, substitute_macro, member_macro \
             ) \
        POTHOS_MPL_HAS_MEMBER_SUBSTITUTE_WITH_TEMPLATE_SFINAE( \
            args, substitute_macro \
        ) \
        POTHOS_MPL_HAS_MEMBER_IMPLEMENTATION( \
            args \
          , POTHOS_MPL_HAS_MEMBER_INTROSPECT_WITH_TEMPLATE_SFINAE \
          , substitute_macro \
          , member_macro \
        ) \
      /**/

#   endif // BOOST_MPL_HAS_XXX_NEEDS_TEMPLATE_SFINAE

// Note: In the current implementation the parameter and access macros
// are no longer expanded.
#   if !POTHOS_WORKAROUND(POTHOS_MSVC, <= 1400)
#     define POTHOS_MPL_HAS_XXX_TEMPLATE_NAMED_DEF(trait, name, default_) \
        POTHOS_MPL_HAS_MEMBER_WITH_FUNCTION_SFINAE( \
            ( 4, ( trait, name, POTHOS_MPL_LIMIT_METAFUNCTION_ARITY, default_ ) ) \
          , POTHOS_MPL_HAS_MEMBER_TEMPLATE_SUBSTITUTE_PARAMETER \
          , POTHOS_MPL_HAS_MEMBER_TEMPLATE_ACCESS \
        ) \
      /**/
#   else
#     define POTHOS_MPL_HAS_XXX_TEMPLATE_NAMED_DEF(trait, name, default_) \
        POTHOS_MPL_HAS_MEMBER_WITH_TEMPLATE_SFINAE( \
            ( 4, ( trait, name, POTHOS_MPL_LIMIT_METAFUNCTION_ARITY, default_ ) ) \
          , POTHOS_MPL_HAS_MEMBER_TEMPLATE_SUBSTITUTE_PARAMETER \
          , POTHOS_MPL_HAS_MEMBER_TEMPLATE_ACCESS \
        ) \
      /**/
#   endif

#else // BOOST_MPL_CFG_NO_HAS_XXX_TEMPLATE

// placeholder implementation

#   define POTHOS_MPL_HAS_XXX_TEMPLATE_NAMED_DEF(trait, name, default_) \
      template< typename T \
              , typename fallback_ = Pothos::mpl::bool_< default_ > > \
      struct trait { \
          POTHOS_STATIC_CONSTANT(bool, value = fallback_::value); \
          typedef fallback_ type; \
      }; \
    /**/

#endif // BOOST_MPL_CFG_NO_HAS_XXX_TEMPLATE

#   define POTHOS_MPL_HAS_XXX_TEMPLATE_DEF(name) \
      POTHOS_MPL_HAS_XXX_TEMPLATE_NAMED_DEF( \
          POTHOS_PP_CAT(has_, name), name, false \
      ) \
    /**/

#endif // BOOST_MPL_HAS_XXX_HPP_INCLUDED
