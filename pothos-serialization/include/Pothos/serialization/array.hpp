#ifndef POTHOS_SERIALIZATION_ARRAY_HPP
#define POTHOS_SERIALIZATION_ARRAY_HPP

// (C) Copyright 2005 Matthias Troyer and Dave Abrahams
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <cstddef> // std::size_t
#include <cstddef>
#include <Pothos/serialization/impl/config.hpp> // msvc 6.0 needs this for warning suppression
#if defined(POTHOS_NO_STDC_NAMESPACE)
namespace std{ 
    using ::size_t; 
} // namespace std
#endif

#include <Pothos/serialization/nvp.hpp>
#include <Pothos/serialization/split_member.hpp>
#include <Pothos/serialization/wrapper.hpp>
#include <Pothos/serialization/impl/mpl/always.hpp>
#include <Pothos/serialization/impl/mpl/apply.hpp>
#include <Pothos/serialization/impl/mpl/bool.hpp>
#include <Pothos/serialization/impl/type_traits/remove_const.hpp>
#include <Pothos/serialization/impl/array.hpp>

namespace Pothos { namespace serialization {

// traits to specify whether to use  an optimized array serialization

#ifdef __BORLANDC__
// workaround for Borland compiler
template <class Archive>
struct use_array_optimization {
  template <class T> struct apply : Pothos::mpl::false_ {};
};

#else
template <class Archive>
struct use_array_optimization : Pothos::mpl::always<Pothos::mpl::false_> {};
#endif

template<class T>
class array :
    public wrapper_traits<const array< T > >
{
public:    
    typedef T value_type;
    
    array(value_type* t, std::size_t s) :
        m_t(t),
        m_element_count(s)
    {}
    array(const array & rhs) :
        m_t(rhs.m_t),
        m_element_count(rhs.m_element_count)
    {}
    array & operator=(const array & rhs){
        m_t = rhs.m_t;
        m_element_count = rhs.m_element_count;
    }

    // default implementation
    template<class Archive>
    void serialize_optimized(Archive &ar, const unsigned int, mpl::false_ ) const
    {
      // default implemention does the loop
      std::size_t c = count();
      value_type * t = address();
      while(0 < c--)
            ar & Pothos::serialization::make_nvp("item", *t++);
    }

    // optimized implementation
    template<class Archive>
    void serialize_optimized(Archive &ar, const unsigned int version, mpl::true_ )
    {
      Pothos::serialization::split_member(ar, *this, version);
    }

    // default implementation
    template<class Archive>
    void save(Archive &ar, const unsigned int version) const
    {
      ar.save_array(*this,version);
    }

    // default implementation
    template<class Archive>
    void load(Archive &ar, const unsigned int version)
    {
      ar.load_array(*this,version);
    }
    
    // default implementation
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
      typedef POTHOS_DEDUCED_TYPENAME 
          Pothos::serialization::use_array_optimization<Archive>::template apply<
                    POTHOS_DEDUCED_TYPENAME remove_const< T >::type 
                >::type use_optimized;
      serialize_optimized(ar,version,use_optimized());
    }
    
    value_type* address() const
    {
      return m_t;
    }

    std::size_t count() const
    {
      return m_element_count;
    }
    
private:
    value_type* m_t;
    std::size_t m_element_count;
};

template<class T>
inline
#ifndef POTHOS_NO_FUNCTION_TEMPLATE_ORDERING
const
#endif
array< T > make_array( T* t, std::size_t s){
    return array< T >(t, s);
}

template <class Archive, class T, std::size_t N>
void serialize(Archive& ar, Pothos::array<T,N>& a, const unsigned int /* version */)
{
  ar & Pothos::serialization::make_nvp("elems",a.elems);
}

} } // end namespace boost::serialization

#ifdef __BORLANDC__
// ignore optimizations for Borland
#define POTHOS_SERIALIZATION_USE_ARRAY_OPTIMIZATION(Archive)      
#else
#define POTHOS_SERIALIZATION_USE_ARRAY_OPTIMIZATION(Archive)           \
namespace Pothos { namespace serialization {                           \
template <> struct use_array_optimization<Archive> {                  \
  template <class ValueType>                                          \
  struct apply : Pothos::mpl::apply1<Archive::use_array_optimization   \
      , POTHOS_DEDUCED_TYPENAME Pothos::remove_const<ValueType>::type   \
    >::type {};                                                       \
}; }}
#endif // __BORLANDC__

#endif //BOOST_SERIALIZATION_ARRAY_HPP
