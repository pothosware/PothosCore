#ifndef POTHOS_ARCHIVE_OSERIALIZER_HPP
#define POTHOS_ARCHIVE_OSERIALIZER_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#pragma inline_depth(511)
#pragma inline_recursion(on)
#endif

#if defined(__MWERKS__)
#pragma inline_depth(511)
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// oserializer.hpp: interface for serialization system.

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

#include <Pothos/serialization/impl/assert.hpp>
#include <cstddef> // NULL

#include <Pothos/serialization/impl/config.hpp>
#include <Pothos/serialization/impl/static_assert.hpp>
#include <Pothos/serialization/impl/detail/workaround.hpp>

#include <Pothos/serialization/impl/mpl/eval_if.hpp>
#include <Pothos/serialization/impl/mpl/equal_to.hpp>
#include <Pothos/serialization/impl/mpl/greater_equal.hpp>
#include <Pothos/serialization/impl/mpl/identity.hpp>

#ifndef POTHOS_SERIALIZATION_DEFAULT_TYPE_INFO   
    #include <Pothos/serialization/extended_type_info_typeid.hpp>   
#endif
#include <Pothos/serialization/throw_exception.hpp>
#include <Pothos/serialization/smart_cast.hpp>
#include <Pothos/serialization/assume_abstract.hpp>
#include <Pothos/serialization/static_warning.hpp>

#include <Pothos/serialization/impl/type_traits/is_pointer.hpp>
#include <Pothos/serialization/impl/type_traits/is_enum.hpp>
#include <Pothos/serialization/impl/type_traits/is_const.hpp>
#include <Pothos/serialization/impl/type_traits/is_polymorphic.hpp>
#include <Pothos/serialization/impl/type_traits/remove_extent.hpp>

#include <Pothos/serialization/serialization.hpp>
#include <Pothos/serialization/version.hpp>
#include <Pothos/serialization/level.hpp>
#include <Pothos/serialization/tracking.hpp>
#include <Pothos/serialization/type_info_implementation.hpp>
#include <Pothos/serialization/nvp.hpp>
#include <Pothos/serialization/void_cast.hpp>
#include <Pothos/serialization/array.hpp>
#include <Pothos/serialization/collection_size_type.hpp>
#include <Pothos/serialization/singleton.hpp>

#include <Pothos/archive/archive_exception.hpp>
#include <Pothos/archive/detail/basic_oarchive.hpp>
#include <Pothos/archive/detail/basic_oserializer.hpp>
#include <Pothos/archive/detail/basic_pointer_oserializer.hpp>
#include <Pothos/archive/detail/archive_serializer_map.hpp>
#include <Pothos/archive/detail/check.hpp>

namespace Pothos {

namespace serialization {
    class extended_type_info;
} // namespace serialization

namespace archive {

// an accessor to permit friend access to archives.  Needed because
// some compilers don't handle friend templates completely
class save_access {
public:
    template<class Archive>
    static void end_preamble(Archive & ar){
        ar.end_preamble();
    }
    template<class Archive, class T>
    static void save_primitive(Archive & ar, const  T & t){
        ar.end_preamble();
        ar.save(t);
    }
};

namespace detail {

#ifdef POTHOS_MSVC
#  pragma warning(push)
#  pragma warning(disable : 4511 4512)
#endif

template<class Archive, class T>
class oserializer : public basic_oserializer
{
private:
    // private constructor to inhibit any existence other than the 
    // static one
public:
    explicit POTHOS_DLLEXPORT oserializer() :
        basic_oserializer(
            Pothos::serialization::singleton<
                POTHOS_DEDUCED_TYPENAME 
                Pothos::serialization::type_info_implementation< T >::type
            >::get_const_instance()
        )
    {}
    virtual POTHOS_DLLEXPORT void save_object_data(
        basic_oarchive & ar,    
        const void *x
    ) const POTHOS_USED;
    virtual bool class_info() const {
        return Pothos::serialization::implementation_level< T >::value 
            >= Pothos::serialization::object_class_info;
    }
    virtual bool tracking(const unsigned int /* flags */) const {
        return Pothos::serialization::tracking_level< T >::value == Pothos::serialization::track_always
            || (Pothos::serialization::tracking_level< T >::value == Pothos::serialization::track_selectively
                && serialized_as_pointer());
    }
    virtual version_type version() const {
        return version_type(::Pothos::serialization::version< T >::value);
    }
    virtual bool is_polymorphic() const {
        return Pothos::is_polymorphic< T >::value;
    }
    virtual ~oserializer(){}
};

#ifdef POTHOS_MSVC
#  pragma warning(pop)
#endif

template<class Archive, class T>
POTHOS_DLLEXPORT void oserializer<Archive, T>::save_object_data(
    basic_oarchive & ar,    
    const void *x
) const {
    // make sure call is routed through the highest interface that might
    // be specialized by the user.
    POTHOS_STATIC_ASSERT(Pothos::is_const< T >::value == false);
    Pothos::serialization::serialize_adl(
        Pothos::serialization::smart_cast_reference<Archive &>(ar),
        * static_cast<T *>(const_cast<void *>(x)),
        version()
    );
}

#ifdef POTHOS_MSVC
#  pragma warning(push)
#  pragma warning(disable : 4511 4512)
#endif

template<class Archive, class T>
class pointer_oserializer :
    public basic_pointer_oserializer
{
private:
    const basic_oserializer & 
    get_basic_serializer() const {
        return Pothos::serialization::singleton<
            oserializer<Archive, T>
        >::get_const_instance();
    }
    virtual POTHOS_DLLEXPORT void save_object_ptr(
        basic_oarchive & ar,
        const void * x
    ) const POTHOS_USED;
public:
    pointer_oserializer();
    ~pointer_oserializer();
};

#ifdef POTHOS_MSVC
#  pragma warning(pop)
#endif

template<class Archive, class T>
POTHOS_DLLEXPORT void pointer_oserializer<Archive, T>::save_object_ptr(
    basic_oarchive & ar,
    const void * x
) const {
    POTHOS_ASSERT(NULL != x);
    // make sure call is routed through the highest interface that might
    // be specialized by the user.
    T * t = static_cast<T *>(const_cast<void *>(x));
    const unsigned int file_version = Pothos::serialization::version< T >::value;
    Archive & ar_impl 
        = Pothos::serialization::smart_cast_reference<Archive &>(ar);
    Pothos::serialization::save_construct_data_adl<Archive, T>(
        ar_impl, 
        t, 
        file_version
    );
    ar_impl << Pothos::serialization::make_nvp(NULL, * t);
}

template<class Archive, class T>
pointer_oserializer<Archive, T>::pointer_oserializer() :
    basic_pointer_oserializer(
        Pothos::serialization::singleton<
            POTHOS_DEDUCED_TYPENAME 
            Pothos::serialization::type_info_implementation< T >::type
        >::get_const_instance()
    )
{
    // make sure appropriate member function is instantiated
    Pothos::serialization::singleton<
        oserializer<Archive, T> 
    >::get_mutable_instance().set_bpos(this);
    archive_serializer_map<Archive>::insert(this);
}

template<class Archive, class T>
pointer_oserializer<Archive, T>::~pointer_oserializer(){
    archive_serializer_map<Archive>::erase(this);
}

template<class Archive>
struct save_non_pointer_type {
    // note this bounces the call right back to the archive
    // with no runtime overhead
    struct save_primitive {
        template<class T>
        static void invoke(Archive & ar, const T & t){
            save_access::save_primitive(ar, t);
        }
    };
    // same as above but passes through serialization
    struct save_only {
        template<class T>
        static void invoke(Archive & ar, const T & t){
            // make sure call is routed through the highest interface that might
            // be specialized by the user.
            Pothos::serialization::serialize_adl(
                ar, 
                const_cast<T &>(t), 
                ::Pothos::serialization::version< T >::value
            );
        }
    };
    // adds class information to the archive. This includes
    // serialization level and class version
    struct save_standard {
        template<class T>
        static void invoke(Archive &ar, const T & t){
            ar.save_object(
                & t, 
                Pothos::serialization::singleton<
                    oserializer<Archive, T>
                >::get_const_instance()
            );
        }
    };

    // adds class information to the archive. This includes
    // serialization level and class version
    struct save_conditional {
        template<class T>
        static void invoke(Archive &ar, const T &t){
            //if(0 == (ar.get_flags() & no_tracking))
                save_standard::invoke(ar, t);
            //else
            //   save_only::invoke(ar, t);
        }
    };


    template<class T>
    static void invoke(Archive & ar, const T & t){
        typedef 
            POTHOS_DEDUCED_TYPENAME mpl::eval_if<
            // if its primitive
                mpl::equal_to<
                    Pothos::serialization::implementation_level< T >,
                    mpl::int_<Pothos::serialization::primitive_type>
                >,
                mpl::identity<save_primitive>,
            // else
            POTHOS_DEDUCED_TYPENAME mpl::eval_if<
                // class info / version
                mpl::greater_equal<
                    Pothos::serialization::implementation_level< T >,
                    mpl::int_<Pothos::serialization::object_class_info>
                >,
                // do standard save
                mpl::identity<save_standard>,
            // else
            POTHOS_DEDUCED_TYPENAME mpl::eval_if<
                    // no tracking
                mpl::equal_to<
                    Pothos::serialization::tracking_level< T >,
                    mpl::int_<Pothos::serialization::track_never>
                >,
                // do a fast save
                mpl::identity<save_only>,
            // else
                // do a fast save only tracking is turned off
                mpl::identity<save_conditional>
            > > >::type typex; 
        check_object_versioning< T >();
        typex::invoke(ar, t);
    }
    template<class T>
    static void invoke(Archive & ar, T & t){
        check_object_level< T >();
        check_object_tracking< T >();
        invoke(ar, const_cast<const T &>(t));
    }
};

template<class Archive>
struct save_pointer_type {
    struct abstract
    {
        template<class T>
        static const basic_pointer_oserializer * register_type(Archive & /* ar */){
            // it has? to be polymorphic
            POTHOS_STATIC_ASSERT(Pothos::is_polymorphic< T >::value);
            return NULL;
        }
    };

    struct non_abstract
    {
        template<class T>
        static const basic_pointer_oserializer * register_type(Archive & ar){
            return ar.register_type(static_cast<T *>(NULL));
        }
    };

    template<class T>
    static const basic_pointer_oserializer * register_type(Archive &ar, T & /*t*/){
        // there should never be any need to save an abstract polymorphic 
        // class pointer.  Inhibiting code generation for this
        // permits abstract base classes to be used - note: exception
        // virtual serialize functions used for plug-ins
        typedef 
            POTHOS_DEDUCED_TYPENAME mpl::eval_if<
                Pothos::serialization::is_abstract< T >,
                mpl::identity<abstract>,
                mpl::identity<non_abstract>       
            >::type typex;
        return typex::template register_type< T >(ar);
    }

    struct non_polymorphic
    {
        template<class T>
        static void save(
            Archive &ar, 
            T & t
        ){
            const basic_pointer_oserializer & bpos = 
                Pothos::serialization::singleton<
                    pointer_oserializer<Archive, T>
                >::get_const_instance();
            // save the requested pointer type
            ar.save_pointer(& t, & bpos);
        }
    };

    struct polymorphic
    {
        template<class T>
        static void save(
            Archive &ar, 
            T & t
        ){
            POTHOS_DEDUCED_TYPENAME 
            Pothos::serialization::type_info_implementation< T >::type const
            & i = Pothos::serialization::singleton<
                POTHOS_DEDUCED_TYPENAME 
                Pothos::serialization::type_info_implementation< T >::type
            >::get_const_instance();

            Pothos::serialization::extended_type_info const * const this_type = & i;

            // retrieve the true type of the object pointed to
            // if this assertion fails its an error in this library
            POTHOS_ASSERT(NULL != this_type);

            const Pothos::serialization::extended_type_info * true_type =
                i.get_derived_extended_type_info(t);

            // note:if this exception is thrown, be sure that derived pointer
            // is either registered or exported.
            if(NULL == true_type){
                Pothos::serialization::throw_exception(
                    archive_exception(
                        archive_exception::unregistered_class,
                        "derived class not registered or exported"
                    )
                );
            }

            // if its not a pointer to a more derived type
            const void *vp = static_cast<const void *>(&t);
            if(*this_type == *true_type){
                const basic_pointer_oserializer * bpos = register_type(ar, t);
                ar.save_pointer(vp, bpos);
                return;
            }
            // convert pointer to more derived type. if this is thrown
            // it means that the base/derived relationship hasn't be registered
            vp = serialization::void_downcast(
                *true_type, 
                *this_type, 
                static_cast<const void *>(&t)
            );
            if(NULL == vp){
                Pothos::serialization::throw_exception(
                    archive_exception(
                        archive_exception::unregistered_cast,
                        true_type->get_debug_info(),
                        this_type->get_debug_info()
                    )
                );
            }

            // since true_type is valid, and this only gets made if the 
            // pointer oserializer object has been created, this should never
            // fail
            const basic_pointer_oserializer * bpos
                = static_cast<const basic_pointer_oserializer *>(
                    Pothos::serialization::singleton<
                        archive_serializer_map<Archive>
                    >::get_const_instance().find(*true_type)
                );
            POTHOS_ASSERT(NULL != bpos);
            if(NULL == bpos)
                Pothos::serialization::throw_exception(
                    archive_exception(
                        archive_exception::unregistered_class,
                        "derived class not registered or exported"
                    )
                );
            ar.save_pointer(vp, bpos);
        }
    };

    template<class T>
    static void save(
        Archive & ar, 
        const T & t
    ){
        check_pointer_level< T >();
        check_pointer_tracking< T >();
        typedef POTHOS_DEDUCED_TYPENAME mpl::eval_if<
            is_polymorphic< T >,
            mpl::identity<polymorphic>,
            mpl::identity<non_polymorphic>
        >::type type;
        type::save(ar, const_cast<T &>(t));
    }

    template<class TPtr>
    static void invoke(Archive &ar, const TPtr t){
        register_type(ar, * t);
        if(NULL == t){
            basic_oarchive & boa 
                = Pothos::serialization::smart_cast_reference<basic_oarchive &>(ar);
            boa.save_null_pointer();
            save_access::end_preamble(ar);
            return;
        }
        save(ar, * t);
    }
};

template<class Archive>
struct save_enum_type
{
    template<class T>
    static void invoke(Archive &ar, const T &t){
        // convert enum to integers on save
        const int i = static_cast<int>(t);
        ar << Pothos::serialization::make_nvp(NULL, i);
    }
};

template<class Archive>
struct save_array_type
{
    template<class T>
    static void invoke(Archive &ar, const T &t){
        typedef POTHOS_DEDUCED_TYPENAME Pothos::remove_extent< T >::type value_type;
        
        save_access::end_preamble(ar);
        // consider alignment
        std::size_t c = sizeof(t) / (
            static_cast<const char *>(static_cast<const void *>(&t[1])) 
            - static_cast<const char *>(static_cast<const void *>(&t[0]))
        );
        Pothos::serialization::collection_size_type count(c);
        ar << POTHOS_SERIALIZATION_NVP(count);
        ar << serialization::make_array(static_cast<value_type const*>(&t[0]),count);
    }
};

} // detail

template<class Archive, class T>
inline void save(Archive & ar, /*const*/ T &t){
    typedef 
        POTHOS_DEDUCED_TYPENAME mpl::eval_if<is_pointer< T >,
            mpl::identity<detail::save_pointer_type<Archive> >,
        //else
        POTHOS_DEDUCED_TYPENAME mpl::eval_if<is_enum< T >,
            mpl::identity<detail::save_enum_type<Archive> >,
        //else
        POTHOS_DEDUCED_TYPENAME mpl::eval_if<is_array< T >,
            mpl::identity<detail::save_array_type<Archive> >,
        //else
            mpl::identity<detail::save_non_pointer_type<Archive> >
        >
        >
        >::type typex;
    typex::invoke(ar, t);
}

} // namespace archive
} // namespace boost

#endif // BOOST_ARCHIVE_OSERIALIZER_HPP
