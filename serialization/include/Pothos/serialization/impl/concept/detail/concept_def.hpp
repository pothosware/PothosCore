// Copyright David Abrahams 2006. Distributed under the Boost
// Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef POTHOS_CONCEPT_DETAIL_CONCEPT_DEF_DWA200651_HPP
# define POTHOS_CONCEPT_DETAIL_CONCEPT_DEF_DWA200651_HPP
# include <Pothos/serialization/impl/preprocessor/seq/for_each_i.hpp>
# include <Pothos/serialization/impl/preprocessor/seq/enum.hpp>
# include <Pothos/serialization/impl/preprocessor/comma_if.hpp>
# include <Pothos/serialization/impl/preprocessor/cat.hpp>
#endif // BOOST_CONCEPT_DETAIL_CONCEPT_DEF_DWA200651_HPP

// BOOST_concept(SomeName, (p1)(p2)...(pN))
//
// Expands to "template <class p1, class p2, ...class pN> struct SomeName"
//
// Also defines an equivalent SomeNameConcept for backward compatibility.
// Maybe in the next release we can kill off the "Concept" suffix for good.
#if POTHOS_WORKAROUND(__GNUC__, <= 3)
# define POTHOS_concept(name, params)                                            \
    template < POTHOS_PP_SEQ_FOR_EACH_I(POTHOS_CONCEPT_typename,~,params) >       \
    struct name; /* forward declaration */                                      \
                                                                                \
    template < POTHOS_PP_SEQ_FOR_EACH_I(POTHOS_CONCEPT_typename,~,params) >       \
    struct POTHOS_PP_CAT(name,Concept)                                           \
      : name< POTHOS_PP_SEQ_ENUM(params) >                                       \
    {                                                                           \
        /* at least 2.96 and 3.4.3 both need this */                            \
        POTHOS_PP_CAT(name,Concept)();                                           \
    };                                                                          \
                                                                                \
    template < POTHOS_PP_SEQ_FOR_EACH_I(POTHOS_CONCEPT_typename,~,params) >       \
    struct name                                                                
#else
# define POTHOS_concept(name, params)                                            \
    template < POTHOS_PP_SEQ_FOR_EACH_I(POTHOS_CONCEPT_typename,~,params) >       \
    struct name; /* forward declaration */                                      \
                                                                                \
    template < POTHOS_PP_SEQ_FOR_EACH_I(POTHOS_CONCEPT_typename,~,params) >       \
    struct POTHOS_PP_CAT(name,Concept)                                           \
      : name< POTHOS_PP_SEQ_ENUM(params) >                                       \
    {                                                                           \
    };                                                                          \
                                                                                \
    template < POTHOS_PP_SEQ_FOR_EACH_I(POTHOS_CONCEPT_typename,~,params) >       \
    struct name                                                                
#endif
    
// Helper for BOOST_concept, above.
# define POTHOS_CONCEPT_typename(r, ignored, index, t) \
    POTHOS_PP_COMMA_IF(index) typename t

