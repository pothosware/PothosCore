// Copyright David Abrahams 2006. Distributed under the Boost
// Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef POTHOS_CONCEPT_CHECK_MSVC_DWA2006429_HPP
# define POTHOS_CONCEPT_CHECK_MSVC_DWA2006429_HPP

# include <Pothos/serialization/impl/preprocessor/cat.hpp>
# include <Pothos/serialization/impl/concept/detail/backward_compatibility.hpp>

# ifdef POTHOS_OLD_CONCEPT_SUPPORT
#  include <Pothos/serialization/impl/concept/detail/has_constraints.hpp>
#  include <Pothos/serialization/impl/mpl/if.hpp>
# endif


namespace Pothos { namespace concepts {


template <class Model>
struct check
{
    virtual void failed(Model* x)
    {
        x->~Model();
    }
};

# ifndef POTHOS_NO_PARTIAL_SPECIALIZATION
struct failed {};
template <class Model>
struct check<failed ************ Model::************>
{
    virtual void failed(Model* x)
    {
        x->~Model();
    }
};
# endif

# ifdef POTHOS_OLD_CONCEPT_SUPPORT
  
namespace detail
{
  // No need for a virtual function here, since evaluating
  // not_satisfied below will have already instantiated the
  // constraints() member.
  struct constraint {};
}

template <class Model>
struct require
  : mpl::if_c<
        not_satisfied<Model>::value
      , detail::constraint
# ifndef POTHOS_NO_PARTIAL_SPECIALIZATION
      , check<Model>
# else
      , check<failed ************ Model::************>
# endif 
        >::type
{};
      
# else
  
template <class Model>
struct require
# ifndef POTHOS_NO_PARTIAL_SPECIALIZATION
    : check<Model>
# else
    : check<failed ************ Model::************>
# endif 
{};
  
# endif
    
# if POTHOS_WORKAROUND(POTHOS_MSVC, == 1310)

//
// The iterator library sees some really strange errors unless we
// do things this way.
//
template <class Model>
struct require<void(*)(Model)>
{
    virtual void failed(Model*)
    {
        require<Model>();
    }
};

# define POTHOS_CONCEPT_ASSERT_FN( ModelFnPtr )      \
enum                                                \
{                                                   \
    POTHOS_PP_CAT(Pothos_concept_check,__LINE__) =    \
    sizeof(::Pothos::concepts::require<ModelFnPtr>)    \
}
  
# else // Not vc-7.1
  
template <class Model>
require<Model>
require_(void(*)(Model));
  
# define POTHOS_CONCEPT_ASSERT_FN( ModelFnPtr )          \
enum                                                    \
{                                                       \
    POTHOS_PP_CAT(Pothos_concept_check,__LINE__) =        \
      sizeof(::Pothos::concepts::require_((ModelFnPtr)0)) \
}
  
# endif
}}

#endif // BOOST_CONCEPT_CHECK_MSVC_DWA2006429_HPP
