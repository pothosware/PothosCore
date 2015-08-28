// Copyright David Abrahams 2009. Distributed under the Boost
// Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef POTHOS_CONCEPT_BACKWARD_COMPATIBILITY_DWA200968_HPP
# define POTHOS_CONCEPT_BACKWARD_COMPATIBILITY_DWA200968_HPP

namespace Pothos
{
  namespace concepts {}

# if defined(POTHOS_HAS_CONCEPTS) && !defined(POTHOS_CONCEPT_NO_BACKWARD_KEYWORD)
  namespace concept = concepts;
# endif 
} // namespace boost::concept

#endif // BOOST_CONCEPT_BACKWARD_COMPATIBILITY_DWA200968_HPP
