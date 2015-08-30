// (C) Copyright Jeremy Siek 2002.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef POTHOS_ITERATOR_CONCEPTS_HPP
#define POTHOS_ITERATOR_CONCEPTS_HPP

#include <Pothos/serialization/impl/concept_check.hpp>
#include <Pothos/serialization/impl/iterator/iterator_categories.hpp>

// Use boost::detail::iterator_traits to work around some MSVC/Dinkumware problems.
#include <Pothos/serialization/impl/detail/iterator.hpp>

#include <Pothos/serialization/impl/type_traits/is_same.hpp>
#include <Pothos/serialization/impl/type_traits/is_integral.hpp>

#include <Pothos/serialization/impl/mpl/bool.hpp>
#include <Pothos/serialization/impl/mpl/if.hpp>
#include <Pothos/serialization/impl/mpl/and.hpp>
#include <Pothos/serialization/impl/mpl/or.hpp>

#include <Pothos/serialization/impl/static_assert.hpp>

// Use boost/limits to work around missing limits headers on some compilers
#include <Pothos/serialization/impl/limits.hpp>
#include <Pothos/serialization/impl/config.hpp>

#include <algorithm>

#include <Pothos/serialization/impl/concept/detail/concept_def.hpp>

namespace Pothos_concepts
{
  // Used a different namespace here (instead of "boost") so that the
  // concept descriptions do not take for granted the names in
  // namespace boost.

  //===========================================================================
  // Iterator Access Concepts

  POTHOS_concept(ReadableIterator,(Iterator))
    : Pothos::Assignable<Iterator>
    , Pothos::CopyConstructible<Iterator>

  {
      typedef POTHOS_DEDUCED_TYPENAME Pothos::detail::iterator_traits<Iterator>::value_type value_type;
      typedef POTHOS_DEDUCED_TYPENAME Pothos::detail::iterator_traits<Iterator>::reference reference;

      POTHOS_CONCEPT_USAGE(ReadableIterator)
      {

          value_type v = *i;
          Pothos::ignore_unused_variable_warning(v);
      }
  private:
      Iterator i;
  };
  
  template <
      typename Iterator
    , typename ValueType = POTHOS_DEDUCED_TYPENAME Pothos::detail::iterator_traits<Iterator>::value_type
  >
  struct WritableIterator
    : Pothos::CopyConstructible<Iterator>
  {
      POTHOS_CONCEPT_USAGE(WritableIterator)
      {
          *i = v;
      }
  private:
      ValueType v;
      Iterator i;
  };

  template <
      typename Iterator
    , typename ValueType = POTHOS_DEDUCED_TYPENAME Pothos::detail::iterator_traits<Iterator>::value_type
  >
  struct WritableIteratorConcept : WritableIterator<Iterator,ValueType> {};
  
  POTHOS_concept(SwappableIterator,(Iterator))
  {
      POTHOS_CONCEPT_USAGE(SwappableIterator)
      {
          std::iter_swap(i1, i2);
      }
  private:
      Iterator i1;
      Iterator i2;
  };

  POTHOS_concept(LvalueIterator,(Iterator))
  {
      typedef typename Pothos::detail::iterator_traits<Iterator>::value_type value_type;
      
      POTHOS_CONCEPT_USAGE(LvalueIterator)
      {
        value_type& r = const_cast<value_type&>(*i);
        Pothos::ignore_unused_variable_warning(r);
      }
  private:
      Iterator i;
  };

  
  //===========================================================================
  // Iterator Traversal Concepts

  POTHOS_concept(IncrementableIterator,(Iterator))
    : Pothos::Assignable<Iterator>
    , Pothos::CopyConstructible<Iterator>
  {
      typedef typename Pothos::iterator_traversal<Iterator>::type traversal_category;

      POTHOS_CONCEPT_ASSERT((
        Pothos::Convertible<
            traversal_category
          , Pothos::incrementable_traversal_tag
        >));

      POTHOS_CONCEPT_USAGE(IncrementableIterator)
      {
          ++i;
          (void)i++;
      }
  private:
      Iterator i;
  };

  POTHOS_concept(SinglePassIterator,(Iterator))
    : IncrementableIterator<Iterator>
    , Pothos::EqualityComparable<Iterator>

  {
      POTHOS_CONCEPT_ASSERT((
          Pothos::Convertible<
             POTHOS_DEDUCED_TYPENAME SinglePassIterator::traversal_category
           , Pothos::single_pass_traversal_tag
          > ));
  };

  POTHOS_concept(ForwardTraversal,(Iterator))
    : SinglePassIterator<Iterator>
    , Pothos::DefaultConstructible<Iterator>
  {
      typedef typename Pothos::detail::iterator_traits<Iterator>::difference_type difference_type;
      
      POTHOS_MPL_ASSERT((Pothos::is_integral<difference_type>));
      POTHOS_MPL_ASSERT_RELATION(std::numeric_limits<difference_type>::is_signed, ==, true);

      POTHOS_CONCEPT_ASSERT((
          Pothos::Convertible<
             POTHOS_DEDUCED_TYPENAME ForwardTraversal::traversal_category
           , Pothos::forward_traversal_tag
          > ));
  };
  
  POTHOS_concept(BidirectionalTraversal,(Iterator))
    : ForwardTraversal<Iterator>
  {
      POTHOS_CONCEPT_ASSERT((
          Pothos::Convertible<
             POTHOS_DEDUCED_TYPENAME BidirectionalTraversal::traversal_category
           , Pothos::bidirectional_traversal_tag
          > ));

      POTHOS_CONCEPT_USAGE(BidirectionalTraversal)
      {
          --i;
          (void)i--;
      }
   private:
      Iterator i;
  };

  POTHOS_concept(RandomAccessTraversal,(Iterator))
    : BidirectionalTraversal<Iterator>
  {
      POTHOS_CONCEPT_ASSERT((
          Pothos::Convertible<
             POTHOS_DEDUCED_TYPENAME RandomAccessTraversal::traversal_category
           , Pothos::random_access_traversal_tag
          > ));

      POTHOS_CONCEPT_USAGE(RandomAccessTraversal)
      {
          i += n;
          i = i + n;
          i = n + i;
          i -= n;
          i = i - n;
          n = i - j;
      }
      
   private:
      typename BidirectionalTraversal<Iterator>::difference_type n;
      Iterator i, j;
  };

  //===========================================================================
  // Iterator Interoperability 

  namespace detail
  {
    template <typename Iterator1, typename Iterator2>
    void interop_single_pass_constraints(Iterator1 const& i1, Iterator2 const& i2)
    {
        bool b;
        b = i1 == i2;
        b = i1 != i2;

        b = i2 == i1;
        b = i2 != i1;
        Pothos::ignore_unused_variable_warning(b);
    }

    template <typename Iterator1, typename Iterator2>
    void interop_rand_access_constraints(
        Iterator1 const& i1, Iterator2 const& i2,
        Pothos::random_access_traversal_tag, Pothos::random_access_traversal_tag)
    {
        bool b;
        typename Pothos::detail::iterator_traits<Iterator2>::difference_type n;
        b = i1 <  i2;
        b = i1 <= i2;
        b = i1 >  i2;
        b = i1 >= i2;
        n = i1 -  i2;

        b = i2 <  i1;
        b = i2 <= i1;
        b = i2 >  i1;
        b = i2 >= i1;
        n = i2 -  i1;
        Pothos::ignore_unused_variable_warning(b);
        Pothos::ignore_unused_variable_warning(n);
    }

    template <typename Iterator1, typename Iterator2>
    void interop_rand_access_constraints(
        Iterator1 const&, Iterator2 const&,
        Pothos::single_pass_traversal_tag, Pothos::single_pass_traversal_tag)
    { }

  } // namespace detail

  POTHOS_concept(InteroperableIterator,(Iterator)(ConstIterator))
  {
   private:
      typedef typename Pothos::detail::pure_traversal_tag<
          typename Pothos::iterator_traversal<
              Iterator
          >::type
      >::type traversal_category;

      typedef typename Pothos::detail::pure_traversal_tag<
          typename Pothos::iterator_traversal<
              ConstIterator
          >::type
      >::type const_traversal_category;
      
  public:
      POTHOS_CONCEPT_ASSERT((SinglePassIterator<Iterator>));
      POTHOS_CONCEPT_ASSERT((SinglePassIterator<ConstIterator>));

      POTHOS_CONCEPT_USAGE(InteroperableIterator)
      {
          detail::interop_single_pass_constraints(i, ci);
          detail::interop_rand_access_constraints(i, ci, traversal_category(), const_traversal_category());

          ci = i;
      }
      
   private:
      Iterator      i;
      ConstIterator ci;
  };

} // namespace boost_concepts

#include <Pothos/serialization/impl/concept/detail/concept_undef.hpp>

#endif // BOOST_ITERATOR_CONCEPTS_HPP
