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
#include <type_traits>
#include <iosfwd>

/*!
 * Register archival functions for a given class.
 * A unique name is required with the class because
 * the typeinfo name is not consistent across platforms.
 */
#define POTHOS_CLASS_EXPORT_GUID(T, id) \
    namespace Pothos { namespace Util { \
        template <> struct ArchiveEntryContainer<T> { \
            static const ArchiveEntry &entry; \
            static void save(std::ostream &os, const T &value) { \
                Pothos::Util::OStreamArchiver<T> archiver(os); \
                Pothos::serialization::serialize(archiver, const_cast<T &>(value), 0); \
            } \
            static void load(std::istream &is, T &value) { \
                Pothos::Util::IStreamArchiver<T> archiver(is); \
                Pothos::serialization::serialize(archiver, value, 0); \
            } \
        }; \
        const ArchiveEntry &ArchiveEntryContainer<T>::entry = \
            ArchiveEntry(typeid(T), id) ; \
    }}

#define POTHOS_SERIALIZATION_SPLIT_FREE(T) \
    namespace Pothos { namespace serialization { \
        template <typename Archive> \
        std::enable_if<std::is_same<OStreamArchiver<T>, Archive>::value>::value \
        serialize(Archive &ar, T &t, const unsigned int ver) { \
            Pothos::serialization::save(ar, t, ver); \
        } \
        template <typename Archive> \
        std::enable_if<std::is_same<IStreamArchiver<T>, Archive>::value>::value \
        serialize(Archive &ar, T &t, const unsigned int ver) { \
            Pothos::serialization::load(ar, t, ver); \
        } \
    }}

namespace Pothos { namespace serialization {
    template <typename Archive, typename T>
    void serialize(Archive &ar, T &t, const unsigned int ver);
}}

namespace Pothos {
namespace Util {

template <typename T>
struct ArchiveEntryContainer;

template <typename T>
struct OStreamArchiver
{
    OStreamArchiver(std::ostream &os):
        os(os)
    {
        return;
    }

    template <typename S>
    void operator&(const S &value)
    {
        Pothos::serialization::serialize(*this, value, 0);
    }

    template <typename S>
    void operator<<(const S &value)
    {
        *this & value;
    }

    std::ostream &os;
};
template <typename T>
struct IStreamArchiver
{
    IStreamArchiver(std::istream &is):
        is(is)
    {
        return;
    }

    template <typename S>
    void operator&(S &value)
    {
        Pothos::serialization::serialize(*this, value, 0);
    }

    template <typename S>
    void operator>>(S &value)
    {
        *this & value;
    }

    std::istream &is;
};

class POTHOS_API ArchiveEntry
{
public:

    ArchiveEntry(const std::type_info &type, const char *id);
};

/*!
 * Serialize a type into the output stream
 */
template <typename T>
void SerializeArchive(std::ostream &os, const T &value)
{
    ArchiveEntryContainer<T>::save(os, value);
}

/*!
 * Deserialize a type into from the input stream
 */
template <typename T>
void DeserializeArchive(std::istream &is, T &value)
{
    ArchiveEntryContainer<T>::load(is, value);
}

} //namespace Util
} //namespace Pothos

