// Copyright David Abrahams 2006. Distributed under the Boost
// Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef POTHOS_CONCEPT_DETAIL_BORLAND_DWA2006429_HPP
# define POTHOS_CONCEPT_DETAIL_BORLAND_DWA2006429_HPP

# include <Pothos/serialization/impl/preprocessor/cat.hpp>
# include <Pothos/serialization/impl/concept/detail/backward_compatibility.hpp>

namespace Pothos { namespace concepts {

template <class ModelFnPtr>
struct require;

template <class Model>
struct require<void(*)(Model)>
{
    enum { instantiate = sizeof((((Model*)0)->~Model()), 3) };
};

#  define POTHOS_CONCEPT_ASSERT_FN( ModelFnPtr )         \
  enum                                                  \
  {                                                     \
      POTHOS_PP_CAT(Pothos_concept_check,__LINE__) =      \
      Pothos::concepts::require<ModelFnPtr>::instantiate  \
  }

}} // namespace boost::concept

#endif // BOOST_CONCEPT_DETAIL_BORLAND_DWA2006429_HPP
