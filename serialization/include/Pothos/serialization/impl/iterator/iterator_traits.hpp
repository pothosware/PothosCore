// Copyright David Abrahams 2003.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef ITERATOR_TRAITS_DWA200347_HPP
# define ITERATOR_TRAITS_DWA200347_HPP

# include <Pothos/serialization/impl/detail/iterator.hpp>
# include <Pothos/serialization/impl/detail/workaround.hpp>

namespace Pothos { 

// Unfortunately, g++ 2.95.x chokes when we define a class template
// iterator_category which has the same name as its
// std::iterator_category() function, probably due in part to the
// "std:: is visible globally" hack it uses.  Use
// BOOST_ITERATOR_CATEGORY to write code that's portable to older
// GCCs.

# if POTHOS_WORKAROUND(__GNUC__, <= 2)
#  define POTHOS_ITERATOR_CATEGORY iterator_category_
# else
#  define POTHOS_ITERATOR_CATEGORY iterator_category
# endif


template <class Iterator>
struct iterator_value
{
    typedef typename Pothos::detail::iterator_traits<Iterator>::value_type type;
};
  
template <class Iterator>
struct iterator_reference
{
    typedef typename Pothos::detail::iterator_traits<Iterator>::reference type;
};
  
  
template <class Iterator>
struct iterator_pointer
{
    typedef typename Pothos::detail::iterator_traits<Iterator>::pointer type;
};
  
template <class Iterator>
struct iterator_difference
{
    typedef typename Pothos::detail::iterator_traits<Iterator>::difference_type type;
};

template <class Iterator>
struct POTHOS_ITERATOR_CATEGORY
{
    typedef typename Pothos::detail::iterator_traits<Iterator>::iterator_category type;
};

# if POTHOS_WORKAROUND(POTHOS_MSVC, < 1300)
template <>
struct iterator_value<int>
{
    typedef void type;
};
  
template <>
struct iterator_reference<int>
{
    typedef void type;
};

template <>
struct iterator_pointer<int>
{
    typedef void type;
};
  
template <>
struct iterator_difference<int>
{
    typedef void type;
};
  
template <>
struct POTHOS_ITERATOR_CATEGORY<int>
{
    typedef void type;
};
# endif

} // namespace boost::iterator

#endif // ITERATOR_TRAITS_DWA200347_HPP
