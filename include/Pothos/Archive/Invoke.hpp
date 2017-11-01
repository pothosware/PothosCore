///
/// \file Archive/Invoke.hpp
///
/// Invoke load/save/serialize without being defined.
/// Relies-on and forces two-phase template resolution.
///
/// \copyright
/// Copyright (c) 2016-2017 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <type_traits>
#include <utility> //std::declval

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

    /*!
     * Check for the presence of a serialize member function for class T.
     * https://stackoverflow.com/questions/34595072/error-with-decltype-template-with-msvc2013
     */
    template <typename T, typename Archive>
    struct hasSerialize
    {
        template <typename U>
        static auto test(U* p) -> decltype(p->serialize(std::declval<Archive&>(), std::declval<const unsigned int>()));

        template <typename U>
        static auto test(...) -> std::false_type;

        using type = typename std::is_same<decltype(test<T>(nullptr)), void>::type;
        static const auto value = type::value;
    };

    /*!
     * Invoke a save operation given an output stream archiver
     */
    template <typename Archive, typename T>
    typename std::enable_if<Archive::isSave::value>::type
    invokeSplit(Archive &ar, T &value, const unsigned int ver)
    {
        const VersionType vt(ver);
        save(ar, value, vt);
    }

    /*!
     * Invoke a load operation given an input stream archiver
     */
    template <typename Archive, typename T>
    typename std::enable_if<!Archive::isSave::value>::type
    invokeSplit(Archive &ar, T &value, const unsigned int ver)
    {
        const VersionType vt(ver);
        load(ar, value, vt);
    }

    /*!
     * Invoke serialize when its not a member function
     */
    template <typename Archive, typename T>
    typename std::enable_if<!hasSerialize<T, Archive>::value>::type
    invokeSerialize(Archive &ar, T &value, const unsigned int ver)
    {
        const VersionType vt(ver);
        serialize(ar, value, vt);
    }

    /*!
     * Invoke serialize when it is a member function
     */
    template <typename Archive, typename T>
    typename std::enable_if<hasSerialize<T, Archive>::value>::type
    invokeSerialize(Archive &ar, T &value, const unsigned int ver)
    {
        value.serialize(ar, ver);
    }

} //namespace serialization
} //namespace Pothos
