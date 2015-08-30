#ifndef POTHOS_SERIALIZATION_SCOPED_PTR_HPP_VP_2003_10_30
#define POTHOS_SERIALIZATION_SCOPED_PTR_HPP_VP_2003_10_30

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

//  Copyright (c) 2003 Vladimir Prus.
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Provides non-intrusive serialization for boost::scoped_ptr
// Does not allow to serialize scoped_ptr's to builtin types.

#include <Pothos/serialization/impl/config.hpp>

#include <Pothos/serialization/impl/scoped_ptr.hpp>
#include <Pothos/serialization/nvp.hpp>
#include <Pothos/serialization/split_free.hpp>

namespace Pothos { 
namespace serialization {
    
    template<class Archive, class T>
    void save(
        Archive & ar, 
        const Pothos::scoped_ptr< T > & t, 
        const unsigned int /* version */
    ){
        T* r = t.get();
        ar << Pothos::serialization::make_nvp("scoped_ptr", r);
    }

    template<class Archive, class T>
    void load(
        Archive & ar, 
        Pothos::scoped_ptr< T > & t, 
        const unsigned int /* version */
    ){
        T* r;
        ar >> Pothos::serialization::make_nvp("scoped_ptr", r);
        t.reset(r); 
    }

    template<class Archive, class T>
    void serialize(
        Archive& ar, 
        Pothos::scoped_ptr< T >& t, 
        const unsigned int version
    ){
        Pothos::serialization::split_free(ar, t, version);
    }

} // namespace serialization
} // namespace boost

#endif // BOOST_SERIALIZATION_SCOPED_PTR_HPP_VP_2003_10_30
