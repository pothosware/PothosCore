// Copyright (C) 2003, Fernando Luis Cacciola Carballal.
// Copyright (C) 2007, Tobias Schwinger.
//
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/optional for documentation.
//
// You are welcome to contact the author at:
//  fernando_cacciola@hotmail.com
//
#ifndef POTHOS_UTILITY_INPLACE_FACTORY_04APR2007_HPP
#ifndef POTHOS_PP_IS_ITERATING

#include <Pothos/serialization/impl/utility/detail/in_place_factory_prefix.hpp>

namespace Pothos {

class in_place_factory_base {} ;

#define  POTHOS_PP_ITERATION_LIMITS (0, POTHOS_MAX_INPLACE_FACTORY_ARITY)
#define  POTHOS_PP_FILENAME_1 <Pothos/serialization/impl/utility/in_place_factory.hpp>
#include POTHOS_PP_ITERATE()

} // namespace boost

#include <Pothos/serialization/impl/utility/detail/in_place_factory_suffix.hpp>

#define POTHOS_UTILITY_INPLACE_FACTORY_04APR2007_HPP
#else
#define N POTHOS_PP_ITERATION()

#if N
template< POTHOS_PP_ENUM_PARAMS(N, class A) >
#endif
class POTHOS_PP_CAT(in_place_factory,N)
  : 
  public in_place_factory_base
{
public:

  explicit POTHOS_PP_CAT(in_place_factory,N)
      ( POTHOS_PP_ENUM_BINARY_PARAMS(N,A,const& a) )
#if N > 0
    : POTHOS_PP_ENUM(N, POTHOS_DEFINE_INPLACE_FACTORY_CLASS_MEMBER_INIT, _)
#endif
  {}

  template<class T>
  void* apply(void* address
      POTHOS_APPEND_EXPLICIT_TEMPLATE_TYPE(T)) const
  {
    return new(address) T( POTHOS_PP_ENUM_PARAMS(N, m_a) );
  }

  template<class T>
  void* apply(void* address, std::size_t n
      POTHOS_APPEND_EXPLICIT_TEMPLATE_TYPE(T)) const
  {
    for(char* next = address = this->POTHOS_NESTED_TEMPLATE apply<T>(address);
        !! --n;)
      this->POTHOS_NESTED_TEMPLATE apply<T>(next = next+sizeof(T));
    return address; 
  }

  POTHOS_PP_REPEAT(N, POTHOS_DEFINE_INPLACE_FACTORY_CLASS_MEMBER_DECL, _)
};

#if N > 0
template< POTHOS_PP_ENUM_PARAMS(N, class A) >
inline POTHOS_PP_CAT(in_place_factory,N)< POTHOS_PP_ENUM_PARAMS(N, A) >
in_place( POTHOS_PP_ENUM_BINARY_PARAMS(N, A, const& a) )
{
  return POTHOS_PP_CAT(in_place_factory,N)< POTHOS_PP_ENUM_PARAMS(N, A) >
      ( POTHOS_PP_ENUM_PARAMS(N, a) );
}
#else
inline in_place_factory0 in_place()
{
  return in_place_factory0();
}
#endif

#undef N
#endif
#endif

