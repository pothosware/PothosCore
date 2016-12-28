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

    /*!
     * Invoke a save operation given an output stream archiver
     */
    template <typename Archive, typename T>
    typename std::enable_if<std::is_same<typename Archive::isSave, std::true_type>::value>::type
    invokeLoadSave(Archive &ar, T &value, const unsigned int ver)
    {
        const VersionType vt(ver);
        save(ar, (const T &)value, vt);
    }

    /*!
     * Invoke a load operation given an input stream archiver
     */
    template <typename Archive, typename T>
    typename std::enable_if<std::is_same<typename Archive::isSave, std::false_type>::value>::type
    invokeLoadSave(Archive &ar, T &value, const unsigned int ver)
    {
        const VersionType vt(ver);
        load(ar, value, vt);
    }

    /*!
     * Invoke serialize (save or load) based on the archive type
     */
    template <typename Archive, typename T>
    void invokeSerialize(Archive &ar, T &value, const unsigned int ver)
    {
        const VersionType vt(ver);
        serialize(ar, value, vt);
    }

} //namespace serialization
} //namespace Pothos
