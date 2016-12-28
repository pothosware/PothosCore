///
/// \file Archive/Invoke.hpp
///
/// Invoke load/save/serialize without being defined.
/// Relies-on and forces two-phase template resolution.
///
/// \copyright
/// Copyright (c) 2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <type_traits>

namespace Pothos {
namespace serialization {

    /*!
     * A type that looks like the version number.
     * This forces two-phase template resolution.
     */
    class VersionType
    {
    public:
        VersionType(const unsigned int ver):
            ver(ver)
        {
            return;
        }

        operator const unsigned int &(void) const
        {
            return ver;
        }

        operator unsigned int &(void)
        {
            return ver;
        }

    private:
        unsigned int ver;
    };

    //https://stackoverflow.com/questions/87372/check-if-a-class-has-a-member-function-of-a-given-signature
    template<typename, typename T>
    struct has_serialize {
        static_assert(
            std::integral_constant<T, false>::value,
            "Second template parameter needs to be of function type.");
    };

    template<typename C, typename Ret, typename... Args>
    class has_serialize<C, Ret(Args...)>
    {
    private:
        template<typename T>
        static constexpr auto check(T*)
        -> typename
            std::is_same<
                decltype( std::declval<T>().serialize( std::declval<Args>()... ) ),
                Ret    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
            >::type;  // attempt to call it and see if the return type is correct

        template<typename>
        static constexpr std::false_type check(...);

        typedef decltype(check<C>(0)) type;

    public:
        static constexpr bool value = type::value;
    };

    /*!
     * Invoke a save operation given an output stream archiver
     */
    template <typename Archive, typename T>
    typename std::enable_if<std::is_same<typename Archive::isSave, std::true_type>::value>::type
    invokeSplit(Archive &ar, T &value, const unsigned int ver)
    {
        const VersionType vt(ver);
        save(ar, value, vt);
    }

    /*!
     * Invoke a load operation given an input stream archiver
     */
    template <typename Archive, typename T>
    typename std::enable_if<std::is_same<typename Archive::isSave, std::false_type>::value>::type
    invokeSplit(Archive &ar, T &value, const unsigned int ver)
    {
        const VersionType vt(ver);
        load(ar, value, vt);
    }

    /*!
     * Invoke serialize when its not a member function
     */
    template <typename Archive, typename T>
    typename std::enable_if<!has_serialize<T, void(Archive &, const unsigned int)>::value>::type
    invokeSerialize(Archive &ar, T &value, const unsigned int ver)
    {
        const VersionType vt(ver);
        serialize(ar, value, vt);
    }

    /*!
     * Invoke serialize when it is a member function
     */
    template <typename Archive, typename T>
    typename std::enable_if<has_serialize<T, void(Archive &, const unsigned int)>::value>::type
    invokeSerialize(Archive &ar, T &value, const unsigned int ver)
    {
        value.serialize(ar, ver);
    }

} //namespace serialization
} //namespace Pothos
