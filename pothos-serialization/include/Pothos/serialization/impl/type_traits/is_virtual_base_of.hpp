//  (C) Copyright Daniel Frey and Robert Ramey 2009.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.
 
#ifndef POTHOS_TT_IS_VIRTUAL_BASE_OF_HPP_INCLUDED
#define POTHOS_TT_IS_VIRTUAL_BASE_OF_HPP_INCLUDED

#include <Pothos/serialization/impl/type_traits/is_base_of.hpp>
#include <Pothos/serialization/impl/type_traits/is_same.hpp>
#include <Pothos/serialization/impl/mpl/and.hpp>
#include <Pothos/serialization/impl/mpl/not.hpp>

// should be the last #include
#include <Pothos/serialization/impl/type_traits/detail/bool_trait_def.hpp>

namespace Pothos {
namespace detail {


#ifdef POTHOS_MSVC
#pragma warning( push )
#pragma warning( disable : 4584 4250)
#elif defined(__GNUC__) && (__GNUC__ >= 4)
#pragma GCC system_header
#endif

template<typename Base, typename Derived, typename tag>
struct is_virtual_base_of_impl
{
    POTHOS_STATIC_CONSTANT(bool, value = false);
};

template<typename Base, typename Derived>
struct is_virtual_base_of_impl<Base, Derived, mpl::true_>
{
#ifdef __BORLANDC__
    struct Pothos_type_traits_internal_struct_X : public virtual Derived, public virtual Base 
    {
       Pothos_type_traits_internal_struct_X();
       Pothos_type_traits_internal_struct_X(const Pothos_type_traits_internal_struct_X&);
       Pothos_type_traits_internal_struct_X& operator=(const Pothos_type_traits_internal_struct_X&);
       ~Pothos_type_traits_internal_struct_X()throw();
    };
    struct Pothos_type_traits_internal_struct_Y : public virtual Derived 
    {
       Pothos_type_traits_internal_struct_Y();
       Pothos_type_traits_internal_struct_Y(const Pothos_type_traits_internal_struct_Y&);
       Pothos_type_traits_internal_struct_Y& operator=(const Pothos_type_traits_internal_struct_Y&);
       ~Pothos_type_traits_internal_struct_Y()throw();
    };
#else
    struct Pothos_type_traits_internal_struct_X : public Derived, virtual Base 
    {
       Pothos_type_traits_internal_struct_X();
       Pothos_type_traits_internal_struct_X(const Pothos_type_traits_internal_struct_X&);
       Pothos_type_traits_internal_struct_X& operator=(const Pothos_type_traits_internal_struct_X&);
       ~Pothos_type_traits_internal_struct_X()throw();
    };
    struct Pothos_type_traits_internal_struct_Y : public Derived 
    {
       Pothos_type_traits_internal_struct_Y();
       Pothos_type_traits_internal_struct_Y(const Pothos_type_traits_internal_struct_Y&);
       Pothos_type_traits_internal_struct_Y& operator=(const Pothos_type_traits_internal_struct_Y&);
       ~Pothos_type_traits_internal_struct_Y()throw();
    };
#endif
    POTHOS_STATIC_CONSTANT(bool, value = (sizeof(Pothos_type_traits_internal_struct_X)==sizeof(Pothos_type_traits_internal_struct_Y)));
};

template<typename Base, typename Derived>
struct is_virtual_base_of_impl2
{
   typedef typename mpl::and_<is_base_of<Base, Derived>, mpl::not_<is_same<Base, Derived> > >::type tag_type;
   typedef is_virtual_base_of_impl<Base, Derived, tag_type> imp;
   POTHOS_STATIC_CONSTANT(bool, value = imp::value);
};

#ifdef POTHOS_MSVC
#pragma warning( pop )
#endif

} // namespace detail

POTHOS_TT_AUX_BOOL_TRAIT_DEF2(
      is_virtual_base_of
       , Base
       , Derived
       , (::Pothos::detail::is_virtual_base_of_impl2<Base,Derived>::value) 
)

#ifndef POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION
POTHOS_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC2_2(typename Base,typename Derived,is_virtual_base_of,Base&,Derived,false)
POTHOS_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC2_2(typename Base,typename Derived,is_virtual_base_of,Base,Derived&,false)
POTHOS_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC2_2(typename Base,typename Derived,is_virtual_base_of,Base&,Derived&,false)
#endif

} // namespace boost

#include <Pothos/serialization/impl/type_traits/detail/bool_trait_undef.hpp>

#endif
