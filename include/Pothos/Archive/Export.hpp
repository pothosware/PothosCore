///
/// \file Archive/Export.hpp
///
/// Serialization registration macros.
///
/// \copyright
/// Copyright (c) 2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Archive/ArchiveEntry.hpp>

/*!
 * Register archival functions for a given class.
 * A unique name is required with the class because
 * the typeinfo name is not consistent across platforms.
 */
#define POTHOS_CLASS_EXPORT_GUID(T, id) \
    namespace Pothos { namespace Archive { \
        template <> struct ArchiveEntryContainer<T> { \
            static const ArchiveEntry &entry; \
        }; \
        const ArchiveEntry &ArchiveEntryContainer<T>::entry = \
            ArchiveEntryT<T>(id) ; \
    }}

//! Register archival functions using the stringified type as the ID
#define POTHOS_CLASS_EXPORT(T) POTHOS_CLASS_EXPORT_GUID(T, #T)

namespace Pothos {
namespace Archive {

template <typename T>
struct ArchiveEntryContainer;

} //namespace Archive
} //namespace Pothos
