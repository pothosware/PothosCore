///
/// \file Archive/Archive.hpp
///
/// Templated serialization API.
///
/// \copyright
/// Copyright (c) 2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <typeinfo>
#include <iosfwd>

/*
namespace Pothos { namespace serialization {
    template <typename Archive, typename T>
    void serialize(Archive &ar, T &t, const unsigned int ver);
}}
*/

namespace Pothos {
namespace Archive {

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
        Pothos::serialization::serialize(OStreamArchiver<S>(os), const_cast<S &>(value), 0);
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
        Pothos::serialization::serialize(IStreamArchiver<S>(is), value, 0);
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
void serializeArchive(std::ostream &os, const T &value)
{
    ArchiveEntryContainer<T>::save(os, value);
}

/*!
 * Deserialize a type into from the input stream
 */
template <typename T>
void deserializeArchive(std::istream &is, T &value)
{
    ArchiveEntryContainer<T>::load(is, value);
}

} //namespace Archive
} //namespace Pothos

