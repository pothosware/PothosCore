///
/// \file Archive/Macros.hpp
///
/// Serialization registration and declaration macros.
///
/// \copyright
/// Copyright (c) 2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Archive/Archive.hpp>
#include <type_traits>
#include <iosfwd>

/*!
 * Register archival functions for a given class.
 * A unique name is required with the class because
 * the typeinfo name is not consistent across platforms.
 */
#define POTHOS_CLASS_EXPORT_GUID(T, id) \
    namespace Pothos { namespace Archive { \
        template <> struct ArchiveEntryContainer<T> { \
            static const ArchiveEntry &entry; \
            static void save(std::ostream &os, const T &value) { \
                Pothos::Archive::OStreamArchiver<T> archiver(os); \
                Pothos::serialization::serialize(archiver, const_cast<T &>(value), 0); \
            } \
            static void load(std::istream &is, T &value) { \
                Pothos::Archive::IStreamArchiver<T> archiver(is); \
                Pothos::serialization::serialize(archiver, value, 0); \
            } \
        }; \
        const ArchiveEntry &ArchiveEntryContainer<T>::entry = \
            ArchiveEntry(typeid(T), id) ; \
    }}

#define POTHOS_SERIALIZATION_SPLIT_FREE(T) \
    namespace Pothos { namespace serialization { \
        template <typename Archive> \
        typename std::enable_if<std::is_same<Pothos::Archive::OStreamArchiver<T>, Archive>::value>::value \
        serialize(Archive &ar, T &t, const unsigned int ver) { \
            Pothos::serialization::save(ar, t, ver); \
        } \
        template <typename Archive> \
        typename std::enable_if<std::is_same<Pothos::Archive::IStreamArchiver<T>, Archive>::value>::value \
        serialize(Archive &ar, T &t, const unsigned int ver) { \
            Pothos::serialization::load(ar, t, ver); \
        } \
    }}
