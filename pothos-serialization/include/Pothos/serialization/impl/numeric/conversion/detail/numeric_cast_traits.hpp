//
//! Copyright (c) 2011-2012
//! Brandon Kohn
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#if !defined(POTHOS_NUMERIC_CONVERSION_DONT_USE_PREPROCESSED_FILES)

    #include <Pothos/serialization/impl/numeric/conversion/detail/preprocessed/numeric_cast_traits_common.hpp>
	
	#if !defined(POTHOS_NO_LONG_LONG)
        #include <Pothos/serialization/impl/numeric/conversion/detail/preprocessed/numeric_cast_traits_long_long.hpp>
	#endif
	
#else
#if !POTHOS_PP_IS_ITERATING

    #include <Pothos/serialization/impl/preprocessor/seq/elem.hpp>
    #include <Pothos/serialization/impl/preprocessor/seq/size.hpp>
    #include <Pothos/serialization/impl/preprocessor/iteration/iterate.hpp>
    
    #if defined(__WAVE__) && defined(POTHOS_NUMERIC_CONVERSION_CREATE_PREPROCESSED_FILES)
        #pragma wave option(preserve: 2, line: 0, output: "preprocessed/numeric_cast_traits_common.hpp")
    #endif
//
//! Copyright (c) 2011-2012
//! Brandon Kohn
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
    #if defined(__WAVE__) && defined(POTHOS_NUMERIC_CONVERSION_CREATE_PREPROCESSED_FILES)
        #pragma wave option(preserve: 1)
    #endif
	
	//! These are the assumed common built in fundamental types (not typedefs/macros.)
	#define POTHOS_NUMERIC_CONVERSION_BASE_BUILTIN_TYPES() \
        (char)                                            \
        (signed char)                                     \
        (unsigned char)                                   \
        (short)                                           \
        (unsigned short)                                  \
        (int)                                             \
        (unsigned int)                                    \
        (long)                                            \
        (unsigned long)                                   \
        (float)                                           \
        (double)                                          \
        (long double)                                     \
    /***/
	
    #define POTHOS_NUMERIC_CONVERSION_SEQ_A() POTHOS_NUMERIC_CONVERSION_BASE_BUILTIN_TYPES()
	#define POTHOS_NUMERIC_CONVERSION_SEQ_B() POTHOS_NUMERIC_CONVERSION_BASE_BUILTIN_TYPES()

namespace Pothos { namespace numeric {

    #define POTHOS_PP_ITERATION_PARAMS_1 (3, (0, POTHOS_PP_DEC(POTHOS_PP_SEQ_SIZE(POTHOS_NUMERIC_CONVERSION_SEQ_A())), <Pothos/serialization/impl/numeric/conversion/detail/numeric_cast_traits.hpp>))
    #include POTHOS_PP_ITERATE()    

}}//namespace boost::numeric;

    #if defined(__WAVE__) && defined(POTHOS_NUMERIC_CONVERSION_CREATE_PREPROCESSED_FILES)
        #pragma wave option(output: null)
    #endif   
	
	#if ( defined(__WAVE__) && defined(POTHOS_NUMERIC_CONVERSION_CREATE_PREPROCESSED_FILES) ) || !defined(POTHOS_NO_LONG_LONG)
	
	    #undef POTHOS_NUMERIC_CONVERSION_SEQ_A
	    #undef POTHOS_NUMERIC_CONVERSION_SEQ_B

	    #if defined(__WAVE__) && defined(POTHOS_NUMERIC_CONVERSION_CREATE_PREPROCESSED_FILES)
            #pragma wave option(preserve: 2, line: 0, output: "preprocessed/numeric_cast_traits_long_long.hpp")
        #endif

//
//! Copyright (c) 2011-2012
//! Brandon Kohn
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
        #if defined(__WAVE__) && defined(POTHOS_NUMERIC_CONVERSION_CREATE_PREPROCESSED_FILES)
            #pragma wave option(preserve: 1)
        #endif

namespace Pothos { namespace numeric {

    #define POTHOS_NUMERIC_CONVERSION_SEQ_A() POTHOS_NUMERIC_CONVERSION_BASE_BUILTIN_TYPES()(Pothos::long_long_type)(Pothos::ulong_long_type)
	#define POTHOS_NUMERIC_CONVERSION_SEQ_B() (Pothos::long_long_type)(Pothos::ulong_long_type)
    
    #define POTHOS_PP_ITERATION_PARAMS_1 (3, (0, POTHOS_PP_DEC(POTHOS_PP_SEQ_SIZE(POTHOS_NUMERIC_CONVERSION_SEQ_A())), <Pothos/serialization/impl/numeric/conversion/detail/numeric_cast_traits.hpp>))
    #include POTHOS_PP_ITERATE()    

}}//namespace boost::numeric;

        #if defined(__WAVE__) && defined(POTHOS_NUMERIC_CONVERSION_CREATE_PREPROCESSED_FILES)
            #pragma wave option(output: null)
        #endif   
	
	#endif
		
    #undef POTHOS_NUMERIC_CONVERSION_BASE_BUILTIN_TYPES
	#undef POTHOS_NUMERIC_CONVERSION_SEQ_A
	#undef POTHOS_NUMERIC_CONVERSION_SEQ_B
    
#elif POTHOS_PP_ITERATION_DEPTH() == 1

    #define POTHOS_PP_ITERATION_PARAMS_2 (3, (0, POTHOS_PP_DEC(POTHOS_PP_SEQ_SIZE(POTHOS_NUMERIC_CONVERSION_SEQ_B())), <Pothos/serialization/impl/numeric/conversion/detail/numeric_cast_traits.hpp>))
    #include POTHOS_PP_ITERATE()

#elif POTHOS_PP_ITERATION_DEPTH() == 2

    //! Generate default traits for the specified source and target.
    #define POTHOS_NUMERIC_CONVERSION_A POTHOS_PP_FRAME_ITERATION(1)
    #define POTHOS_NUMERIC_CONVERSION_B POTHOS_PP_FRAME_ITERATION(2)

    template <>
    struct numeric_cast_traits
        <
            POTHOS_PP_SEQ_ELEM(POTHOS_NUMERIC_CONVERSION_A, POTHOS_NUMERIC_CONVERSION_SEQ_A())
          , POTHOS_PP_SEQ_ELEM(POTHOS_NUMERIC_CONVERSION_B, POTHOS_NUMERIC_CONVERSION_SEQ_B())
        >
    {
        typedef def_overflow_handler overflow_policy;
        typedef UseInternalRangeChecker range_checking_policy;
        typedef Trunc<POTHOS_PP_SEQ_ELEM(POTHOS_NUMERIC_CONVERSION_B, POTHOS_NUMERIC_CONVERSION_SEQ_B())> rounding_policy;
    };     

    #undef POTHOS_NUMERIC_CONVERSION_A
    #undef POTHOS_NUMERIC_CONVERSION_B

#endif//! Depth 2.
#endif// BOOST_NUMERIC_CONVERSION_DONT_USE_PREPROCESSED_FILES
