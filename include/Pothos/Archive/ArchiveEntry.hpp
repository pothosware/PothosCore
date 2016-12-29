///
/// \file Archive/ArchiveEntry.hpp
///
/// Library storage for archive entries.
/// Entries are used for polymorphic factories.
///
/// \copyright
/// Copyright (c) 2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Archive/StreamArchiver.hpp>
#include <type_traits>
#include <typeinfo>
#include <iosfwd>
#include <string>

namespace Pothos {
namespace Archive {

/*!
 * Base class for an archive entry.
 * Stores and ID and overloads for polymorphic support.
 */
class POTHOS_API ArchiveEntry
{
public:

    //! Create and register an entry given the type and unique ID
    ArchiveEntry(const std::type_info &type, const std::string &id);

    //! Save a pointer to the archive in a derived class
    virtual void save(void *ar, void *t) const = 0;

    //! Load a pointer from the archive in a derived class
    virtual void *load(void *ar) const = 0;

    //! Lookup the entry given the type info or throw if not found
    static const ArchiveEntry &find(const std::type_info &type);

    //! Lookup the entry given the unique id or throw if not found
    static const ArchiveEntry &find(const std::string &id);

    //! Lookup the entry given the the id hash or throw if not found
    static const ArchiveEntry &find(const unsigned long long &hash);

    //! Get the associated unique ID
    const std::string &getId(void) const;

    //! Get a reproducible hash for this entry
    const unsigned long long &getHash(void) const;

private:
    const std::string _id;
    const unsigned long long _hash;
};

/*!
 * Archive entry for specific types.
 * When instantiated, the library can
 * save, create, and load this type.
 */
template <typename T>
struct ArchiveEntryT : ArchiveEntry
{
    ArchiveEntryT(const std::string &id);

    void save(void *ar, void *t) const;

    void *load(void *ar) const;
};

} //namespace Archive
} //namespace Pothos

inline const std::string &Pothos::Archive::ArchiveEntry::getId(void) const
{
    return _id;
}

inline const unsigned long long &Pothos::Archive::ArchiveEntry::getHash(void) const
{
    return _hash;
}

template <typename T>
Pothos::Archive::ArchiveEntryT<T>::ArchiveEntryT(const std::string &id):
    ArchiveEntry(typeid(T), id)
{
    return;
}

template <typename T>
void Pothos::Archive::ArchiveEntryT<T>::save(void *ar, void *t) const
{
    (*static_cast<OStreamArchiver *>(ar)) << (*static_cast<T *>(t));
}

template <typename T>
void *Pothos::Archive::ArchiveEntryT<T>::load(void *ar) const
{
    T *t = new T();
    (*static_cast<IStreamArchiver *>(ar)) >> *t;
    return t;
}
