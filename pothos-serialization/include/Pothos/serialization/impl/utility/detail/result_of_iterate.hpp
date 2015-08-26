// Boost result_of library

//  Copyright Douglas Gregor 2004. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

//  Copyright Daniel Walker, Eric Niebler, Michel Morin 2008-2012.
//  Use, modification and distribution is subject to the Boost Software
//  License, Version 1.0. (See accompanying file LICENSE_1_0.txt or
//  copy at http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org/libs/utility
#if !defined(POTHOS_PP_IS_ITERATING)
# error Boost result_of - do not include this file!
#endif

// CWPro8 requires an argument in a function type specialization
#if POTHOS_WORKAROUND(__MWERKS__, POTHOS_TESTED_AT(0x3002)) && POTHOS_PP_ITERATION() == 0
# define POTHOS_RESULT_OF_ARGS void
#else
# define POTHOS_RESULT_OF_ARGS POTHOS_PP_ENUM_PARAMS(POTHOS_PP_ITERATION(),T)
#endif

#if !POTHOS_WORKAROUND(__BORLANDC__, POTHOS_TESTED_AT(0x551))
template<typename F POTHOS_PP_ENUM_TRAILING_PARAMS(POTHOS_PP_ITERATION(),typename T)>
struct tr1_result_of<F(POTHOS_RESULT_OF_ARGS)>
    : mpl::if_<
          mpl::or_< is_pointer<F>, is_member_function_pointer<F> >
        , Pothos::detail::tr1_result_of_impl<
            typename remove_cv<F>::type,
            typename remove_cv<F>::type(POTHOS_RESULT_OF_ARGS),
            (Pothos::detail::has_result_type<F>::value)>
        , Pothos::detail::tr1_result_of_impl<
            F,
            F(POTHOS_RESULT_OF_ARGS),
            (Pothos::detail::has_result_type<F>::value)> >::type { };
#endif

#ifdef POTHOS_RESULT_OF_USE_DECLTYPE
template<typename F POTHOS_PP_ENUM_TRAILING_PARAMS(POTHOS_PP_ITERATION(),typename T)>
struct result_of<F(POTHOS_RESULT_OF_ARGS)>
    : detail::cpp0x_result_of<F(POTHOS_RESULT_OF_ARGS)> { };
#endif // BOOST_RESULT_OF_USE_DECLTYPE

#ifdef POTHOS_RESULT_OF_USE_TR1_WITH_DECLTYPE_FALLBACK
template<typename F POTHOS_PP_ENUM_TRAILING_PARAMS(POTHOS_PP_ITERATION(),typename T)>
struct result_of<F(POTHOS_RESULT_OF_ARGS)>
    : mpl::if_<mpl::or_<detail::has_result_type<F>, detail::has_result<F> >,
               tr1_result_of<F(POTHOS_RESULT_OF_ARGS)>,
               detail::cpp0x_result_of<F(POTHOS_RESULT_OF_ARGS)> >::type { };
#endif // BOOST_RESULT_OF_USE_TR1_WITH_DECLTYPE_FALLBACK

#if defined(POTHOS_RESULT_OF_USE_DECLTYPE) || defined(POTHOS_RESULT_OF_USE_TR1_WITH_DECLTYPE_FALLBACK)

namespace detail {

template<typename F POTHOS_PP_ENUM_TRAILING_PARAMS(POTHOS_PP_ITERATION(),typename T)>
struct cpp0x_result_of<F(POTHOS_PP_ENUM_PARAMS(POTHOS_PP_ITERATION(),T))>
    : mpl::if_<
          is_member_function_pointer<F>
        , detail::tr1_result_of_impl<
            typename remove_cv<F>::type,
            typename remove_cv<F>::type(POTHOS_PP_ENUM_PARAMS(POTHOS_PP_ITERATION(),T)), false
          >
        , detail::cpp0x_result_of_impl<
              F(POTHOS_PP_ENUM_PARAMS(POTHOS_PP_ITERATION(),T))
          >
      >::type
{};

#ifdef POTHOS_NO_SFINAE_EXPR

template<typename F>
struct POTHOS_PP_CAT(result_of_callable_fun_2_, POTHOS_PP_ITERATION());

template<typename R POTHOS_PP_ENUM_TRAILING_PARAMS(POTHOS_PP_ITERATION(), typename T)>
struct POTHOS_PP_CAT(result_of_callable_fun_2_, POTHOS_PP_ITERATION())<R(POTHOS_PP_ENUM_PARAMS(POTHOS_PP_ITERATION(), T))> {
    R operator()(POTHOS_PP_ENUM_PARAMS(POTHOS_PP_ITERATION(), T)) const;
    typedef result_of_private_type const &(*pfn_t)(...);
    operator pfn_t() const volatile;
};

template<typename F>
struct POTHOS_PP_CAT(result_of_callable_fun_, POTHOS_PP_ITERATION());

template<typename F>
struct POTHOS_PP_CAT(result_of_callable_fun_, POTHOS_PP_ITERATION())<F *>
  : POTHOS_PP_CAT(result_of_callable_fun_2_, POTHOS_PP_ITERATION())<F>
{};

template<typename F>
struct POTHOS_PP_CAT(result_of_callable_fun_, POTHOS_PP_ITERATION())<F &>
  : POTHOS_PP_CAT(result_of_callable_fun_2_, POTHOS_PP_ITERATION())<F>
{};

template<typename F>
struct POTHOS_PP_CAT(result_of_select_call_wrapper_type_, POTHOS_PP_ITERATION())
  : mpl::eval_if<
        is_class<typename remove_reference<F>::type>,
        result_of_wrap_callable_class<F>,
        mpl::identity<POTHOS_PP_CAT(result_of_callable_fun_, POTHOS_PP_ITERATION())<typename remove_cv<F>::type> >
    >
{};

template<typename F POTHOS_PP_ENUM_TRAILING_PARAMS(POTHOS_PP_ITERATION(), typename T)>
struct POTHOS_PP_CAT(result_of_is_callable_, POTHOS_PP_ITERATION()) {
    typedef typename POTHOS_PP_CAT(result_of_select_call_wrapper_type_, POTHOS_PP_ITERATION())<F>::type wrapper_t;
    static const bool value = (
        sizeof(result_of_no_type) == sizeof(detail::result_of_is_private_type(
            (Pothos::declval<wrapper_t>()(POTHOS_PP_ENUM_BINARY_PARAMS(POTHOS_PP_ITERATION(), Pothos::declval<T, >() POTHOS_PP_INTERCEPT)), result_of_weird_type())
        ))
    );
    typedef mpl::bool_<value> type;
};

template<typename F POTHOS_PP_ENUM_TRAILING_PARAMS(POTHOS_PP_ITERATION(),typename T)>
struct cpp0x_result_of_impl<F(POTHOS_PP_ENUM_PARAMS(POTHOS_PP_ITERATION(),T)), true>
    : lazy_enable_if<
          POTHOS_PP_CAT(result_of_is_callable_, POTHOS_PP_ITERATION())<F POTHOS_PP_ENUM_TRAILING_PARAMS(POTHOS_PP_ITERATION(), T)>
        , cpp0x_result_of_impl<F(POTHOS_PP_ENUM_PARAMS(POTHOS_PP_ITERATION(),T)), false>
      >
{};

template<typename F POTHOS_PP_ENUM_TRAILING_PARAMS(POTHOS_PP_ITERATION(),typename T)>
struct cpp0x_result_of_impl<F(POTHOS_PP_ENUM_PARAMS(POTHOS_PP_ITERATION(),T)), false>
{
  typedef decltype(
    Pothos::declval<F>()(
      POTHOS_PP_ENUM_BINARY_PARAMS(POTHOS_PP_ITERATION(), Pothos::declval<T, >() POTHOS_PP_INTERCEPT)
    )
  ) type;
};

#else // BOOST_NO_SFINAE_EXPR

template<typename F POTHOS_PP_ENUM_TRAILING_PARAMS(POTHOS_PP_ITERATION(),typename T)>
struct cpp0x_result_of_impl<F(POTHOS_PP_ENUM_PARAMS(POTHOS_PP_ITERATION(),T)),
                            typename result_of_always_void<decltype(
                                Pothos::declval<F>()(
                                    POTHOS_PP_ENUM_BINARY_PARAMS(POTHOS_PP_ITERATION(), Pothos::declval<T, >() POTHOS_PP_INTERCEPT)
                                )
                            )>::type> {
  typedef decltype(
    Pothos::declval<F>()(
      POTHOS_PP_ENUM_BINARY_PARAMS(POTHOS_PP_ITERATION(), Pothos::declval<T, >() POTHOS_PP_INTERCEPT)
    )
  ) type;
};

#endif // BOOST_NO_SFINAE_EXPR

} // namespace detail

#else // defined(BOOST_RESULT_OF_USE_DECLTYPE) || defined(BOOST_RESULT_OF_USE_TR1_WITH_DECLTYPE_FALLBACK)

#if !POTHOS_WORKAROUND(__BORLANDC__, POTHOS_TESTED_AT(0x551))
template<typename F POTHOS_PP_ENUM_TRAILING_PARAMS(POTHOS_PP_ITERATION(),typename T)>
struct result_of<F(POTHOS_RESULT_OF_ARGS)>
    : tr1_result_of<F(POTHOS_RESULT_OF_ARGS)> { };
#endif

#endif // defined(BOOST_RESULT_OF_USE_DECLTYPE)

#undef POTHOS_RESULT_OF_ARGS

#if POTHOS_PP_ITERATION() >= 1

namespace detail {

template<typename R,  typename FArgs POTHOS_PP_ENUM_TRAILING_PARAMS(POTHOS_PP_ITERATION(),typename T)>
struct tr1_result_of_impl<R (*)(POTHOS_PP_ENUM_PARAMS(POTHOS_PP_ITERATION(),T)), FArgs, false>
{
  typedef R type;
};

template<typename R,  typename FArgs POTHOS_PP_ENUM_TRAILING_PARAMS(POTHOS_PP_ITERATION(),typename T)>
struct tr1_result_of_impl<R (&)(POTHOS_PP_ENUM_PARAMS(POTHOS_PP_ITERATION(),T)), FArgs, false>
{
  typedef R type;
};

#if !POTHOS_WORKAROUND(__BORLANDC__, POTHOS_TESTED_AT(0x551))
template<typename R, typename FArgs POTHOS_PP_ENUM_TRAILING_PARAMS(POTHOS_PP_ITERATION(),typename T)>
struct tr1_result_of_impl<R (T0::*)
                     (POTHOS_PP_ENUM_SHIFTED_PARAMS(POTHOS_PP_ITERATION(),T)),
                 FArgs, false>
{
  typedef R type;
};

template<typename R, typename FArgs POTHOS_PP_ENUM_TRAILING_PARAMS(POTHOS_PP_ITERATION(),typename T)>
struct tr1_result_of_impl<R (T0::*)
                     (POTHOS_PP_ENUM_SHIFTED_PARAMS(POTHOS_PP_ITERATION(),T))
                     const,
                 FArgs, false>
{
  typedef R type;
};

template<typename R, typename FArgs POTHOS_PP_ENUM_TRAILING_PARAMS(POTHOS_PP_ITERATION(),typename T)>
struct tr1_result_of_impl<R (T0::*)
                     (POTHOS_PP_ENUM_SHIFTED_PARAMS(POTHOS_PP_ITERATION(),T))
                     volatile,
                 FArgs, false>
{
  typedef R type;
};

template<typename R, typename FArgs POTHOS_PP_ENUM_TRAILING_PARAMS(POTHOS_PP_ITERATION(),typename T)>
struct tr1_result_of_impl<R (T0::*)
                     (POTHOS_PP_ENUM_SHIFTED_PARAMS(POTHOS_PP_ITERATION(),T))
                     const volatile,
                 FArgs, false>
{
  typedef R type;
};
#endif

}
#endif
