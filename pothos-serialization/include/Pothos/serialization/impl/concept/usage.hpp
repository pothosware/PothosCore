// Copyright David Abrahams 2006. Distributed under the Boost
// Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef POTHOS_CONCEPT_USAGE_DWA2006919_HPP
# define POTHOS_CONCEPT_USAGE_DWA2006919_HPP

# include <Pothos/serialization/impl/concept/assert.hpp>
# include <Pothos/serialization/impl/detail/workaround.hpp>
# include <Pothos/serialization/impl/concept/detail/backward_compatibility.hpp>

namespace Pothos { namespace concepts { 

# if POTHOS_WORKAROUND(__GNUC__, == 2)

#  define POTHOS_CONCEPT_USAGE(model) ~model()

# else 

template <class Model>
struct usage_requirements
{
    ~usage_requirements() { ((Model*)0)->~Model(); }
};

#  if POTHOS_WORKAROUND(__GNUC__, <= 3)

#   define POTHOS_CONCEPT_USAGE(model)                                    \
      model(); /* at least 2.96 and 3.4.3 both need this :( */           \
      POTHOS_CONCEPT_ASSERT((Pothos::concepts::usage_requirements<model>)); \
      ~model()

#  else

#   define POTHOS_CONCEPT_USAGE(model)                                    \
      POTHOS_CONCEPT_ASSERT((Pothos::concepts::usage_requirements<model>)); \
      ~model()

#  endif

# endif 

}} // namespace boost::concepts

#endif // BOOST_CONCEPT_USAGE_DWA2006919_HPP
