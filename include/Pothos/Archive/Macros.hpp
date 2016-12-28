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
#include <Pothos/Archive/Invoke.hpp>

/*!
 * Register archival functions for a given class.
 * A unique name is required with the class because
 * the typeinfo name is not consistent across platforms.
 */
#define POTHOS_CLASS_EXPORT_ID(T, id) \
    namespace Pothos { namespace Archive { \
        template <> struct ArchiveEntryContainer<T> { \
            static const ArchiveEntry &entry; \
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
        void serialize(Archive &ar, T &t, const unsigned int ver) { \
            Pothos::serialization::invokeLoadSave(ar, t, ver); \
        } \
    }}

/*!
 * Declare a serialize() function that can dispatch to
 * individually declared save and load member functions.
 */
#define POTHOS_SERIALIZATION_SPLIT_MEMBER(T) \
    namespace Pothos { namespace serialization { \
        template <typename Archive> \
        void serialize(Archive &ar, T &t, const unsigned int ver) { \
            Pothos::serialization::invokeLoadSaveMember(ar, t, ver); \
        } \
    }}
