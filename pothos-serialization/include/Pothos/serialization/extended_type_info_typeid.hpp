#ifndef POTHOS_SERIALIZATION_EXTENDED_TYPE_INFO_TYPEID_HPP
#define POTHOS_SERIALIZATION_EXTENDED_TYPE_INFO_TYPEID_HPP

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// extended_type_info_typeid.hpp: implementation for version that depends
// on runtime typing (rtti - typeid) but uses a user specified string
// as the portable class identifier.

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

#include <typeinfo>
#include <cstdarg>
#include <Pothos/serialization/impl/assert.hpp>
#include <Pothos/serialization/impl/config.hpp>

#include <Pothos/serialization/impl/static_assert.hpp>
#include <Pothos/serialization/static_warning.hpp>
#include <Pothos/serialization/impl/type_traits/is_polymorphic.hpp>
#include <Pothos/serialization/impl/type_traits/remove_const.hpp>

#include <Pothos/serialization/config.hpp>
#include <Pothos/serialization/singleton.hpp>
#include <Pothos/serialization/extended_type_info.hpp>
#include <Pothos/serialization/factory.hpp>

// hijack serialization access
#include <Pothos/serialization/access.hpp>

#include <Pothos/serialization/impl/mpl/if.hpp>

#include <Pothos/serialization/impl/config/abi_prefix.hpp> // must be the last header

#ifdef POTHOS_MSVC
#  pragma warning(push)
#  pragma warning(disable : 4251 4231 4660 4275 4511 4512)
#endif

namespace Pothos {
namespace serialization {
namespace typeid_system {

class POTHOS_SERIALIZATION_DECL(POTHOS_PP_EMPTY()) extended_type_info_typeid_0 : 
    public extended_type_info
{
    virtual const char * get_debug_info() const {
        if(static_cast<const std::type_info *>(0) == m_ti)
            return static_cast<const char *>(0);
        return m_ti->name();
    }
protected:
    const std::type_info * m_ti;
    extended_type_info_typeid_0(const char * key);
    ~extended_type_info_typeid_0();
    void type_register(const std::type_info & ti);
    void type_unregister();
    const extended_type_info *
    get_extended_type_info(const std::type_info & ti) const;
public:
    virtual bool
    is_less_than(const extended_type_info &rhs) const;
    virtual bool
    is_equal(const extended_type_info &rhs) const;
    const std::type_info & get_typeid() const {
        return *m_ti;
    }
};

} // typeid_system

template<class T>
class extended_type_info_typeid : 
    public typeid_system::extended_type_info_typeid_0,
    public singleton<extended_type_info_typeid< T > >
{
public:
    extended_type_info_typeid() :
        typeid_system::extended_type_info_typeid_0(get_key())
    {
        type_register(typeid(T));
        key_register();
    }
    ~extended_type_info_typeid(){
        key_unregister();
        type_unregister();
    }
    // get the eti record for the true type of this record
    // relying upon standard type info implemenation (rtti)
    const extended_type_info *
    get_derived_extended_type_info(const T & t) const {
        // note: this implementation - based on usage of typeid (rtti)
        // only does something if the class has at least one virtual function.
        POTHOS_STATIC_WARNING(Pothos::is_polymorphic< T >::value);
        return 
            typeid_system::extended_type_info_typeid_0::get_extended_type_info(
                typeid(t)
            );
    }
    const char * get_key() const {
        return Pothos::serialization::guid< T >();
    }
    virtual void * construct(unsigned int count, ...) const{
        // count up the arguments
        std::va_list ap;
        va_start(ap, count);
        switch(count){
        case 0:
            return factory<POTHOS_DEDUCED_TYPENAME Pothos::remove_const< T >::type, 0>(ap);
        case 1:
            return factory<POTHOS_DEDUCED_TYPENAME Pothos::remove_const< T >::type, 1>(ap);
        case 2:
            return factory<POTHOS_DEDUCED_TYPENAME Pothos::remove_const< T >::type, 2>(ap);
        case 3:
            return factory<POTHOS_DEDUCED_TYPENAME Pothos::remove_const< T >::type, 3>(ap);
        case 4:
            return factory<POTHOS_DEDUCED_TYPENAME Pothos::remove_const< T >::type, 4>(ap);
        default:
            POTHOS_ASSERT(false); // too many arguments
            // throw exception here?
            return NULL;
        }
    }
    virtual void destroy(void const * const p) const {
        Pothos::serialization::access::destroy(
            static_cast<T const *>(p)
        );
        //delete static_cast<T const * const>(p);
    }
};

} // namespace serialization
} // namespace boost

///////////////////////////////////////////////////////////////////////////////
// If no other implementation has been designated as default, 
// use this one.  To use this implementation as the default, specify it
// before any of the other headers.
#ifndef POTHOS_SERIALIZATION_DEFAULT_TYPE_INFO
    #define POTHOS_SERIALIZATION_DEFAULT_TYPE_INFO
    namespace Pothos {
    namespace serialization {
    template<class T>
    struct extended_type_info_impl {
        typedef POTHOS_DEDUCED_TYPENAME 
            Pothos::serialization::extended_type_info_typeid< T > type;
    };
    } // namespace serialization
    } // namespace boost
#endif

#ifdef POTHOS_MSVC
#pragma warning(pop)
#endif
#include <Pothos/serialization/impl/config/abi_suffix.hpp> // pops abi_suffix.hpp pragmas

#endif // BOOST_SERIALIZATION_EXTENDED_TYPE_INFO_TYPEID_HPP
