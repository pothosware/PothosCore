// Copyright David Abrahams 2006. Distributed under the Boost
// Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef POTHOS_CONCEPT_DETAIL_GENERAL_DWA2006429_HPP
# define POTHOS_CONCEPT_DETAIL_GENERAL_DWA2006429_HPP

# include <Pothos/serialization/impl/preprocessor/cat.hpp>
# include <Pothos/serialization/impl/concept/detail/backward_compatibility.hpp>

# ifdef POTHOS_OLD_CONCEPT_SUPPORT
#  include <Pothos/serialization/impl/concept/detail/has_constraints.hpp>
#  include <Pothos/serialization/impl/mpl/if.hpp>
# endif

// This implementation works on Comeau and GCC, all the way back to
// 2.95
namespace Pothos { namespace concepts {

template <class ModelFn>
struct requirement_;

namespace detail
{
  template <void(*)()> struct instantiate {};
}

template <class Model>
struct requirement
{
    static void failed() { ((Model*)0)->~Model(); }
};

struct failed {};

template <class Model>
struct requirement<failed ************ Model::************>
{
    static void failed() { ((Model*)0)->~Model(); }
};

# ifdef POTHOS_OLD_CONCEPT_SUPPORT

template <class Model>
struct constraint
{
    static void failed() { ((Model*)0)->constraints(); }
};
  
template <class Model>
struct requirement_<void(*)(Model)>
  : mpl::if_<
        concepts::not_satisfied<Model>
      , constraint<Model>
      , requirement<failed ************ Model::************>
    >::type
{};
  
# else

// For GCC-2.x, these can't have exactly the same name
template <class Model>
struct requirement_<void(*)(Model)>
    : requirement<failed ************ Model::************>
{};
  
# endif

#  define POTHOS_CONCEPT_ASSERT_FN( ModelFnPtr )             \
    typedef ::Pothos::concepts::detail::instantiate<          \
    &::Pothos::concepts::requirement_<ModelFnPtr>::failed>    \
      POTHOS_PP_CAT(Pothos_concept_check,__LINE__)

}}

#endif // BOOST_CONCEPT_DETAIL_GENERAL_DWA2006429_HPP
