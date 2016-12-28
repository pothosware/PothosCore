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
#define POTHOS_CLASS_EXPORT_ID(T, id) \
    namespace Pothos { namespace Archive { \
        template <> struct ArchiveEntryContainer<T> { \
            static const ArchiveEntry &entry; \
            static void save(OStreamArchiver &ar, const T &value, const int ver) { \
                Pothos::serialization::serialize(ar, const_cast<T &>(value), ver); \
            } \
            static void load(IStreamArchiver &ar, T &value, const int ver) { \
                Pothos::serialization::serialize(ar, value, ver); \
            } \
        }; \
        const ArchiveEntry &ArchiveEntryContainer<T>::entry = \
            ArchiveEntry(typeid(T), id) ; \
    }}

//! Register archival functions using the stringified type as the ID
#define POTHOS_CLASS_EXPORT(T) POTHOS_CLASS_EXPORT_ID(T, #T)

/*!
 * Declare a serialize() function that can dispatch to
 * an individually declared save and load function.
 */
#define POTHOS_SERIALIZATION_SPLIT_FREE(T) \
    namespace Pothos { namespace serialization { \
        template <typename Archive> \
        typename std::enable_if<std::is_same<Pothos::Archive::OStreamArchiver, Archive>::value>::type \
        serialize(Archive &ar, T &t, const unsigned int ver) { \
            Pothos::serialization::save(ar, t, ver); \
        } \
        template <typename Archive> \
        typename std::enable_if<std::is_same<Pothos::Archive::IStreamArchiver, Archive>::value>::type \
        serialize(Archive &ar, T &t, const unsigned int ver) { \
            Pothos::serialization::load(ar, t, ver); \
        } \
    }}
