///
/// \file Util/Archive.hpp
///
/// Serialization registry and utility functions.
///
/// \copyright
/// Copyright (c) 2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <typeinfo>

/*!
 * Register archival functions for a given class.
 * A unique name is required with the class because
 * the typeinfo name is not consistent across platforms.
 */
#define POTHOS_CLASS_EXPORT_GUID(T, id) \
    namespace Pothos { namespace Util { \
        template <> struct ArchiveEntryContainer<T> { \
            static const ArchiveEntry &entry; \
        }; \
        const ArchiveEntry &ArchiveEntryContainer<T>::entry = \
            ArchiveEntry(typeid(T), id) ; \
    }}

namespace Pothos {
namespace Util {

template <typename T>
struct ArchiveEntryContainer;

class POTHOS_API ArchiveEntry
{
public:

    ArchiveEntry(const std::type_info &type, const char *id);
};

} //namespace Util
} //namespace Pothos
