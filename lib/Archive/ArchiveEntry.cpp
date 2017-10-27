// Copyright (c) 2016-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Archive/ArchiveEntry.hpp>
#include <Pothos/Archive/Exception.hpp>
#include <Pothos/Util/TypeInfo.hpp>
#include <map>

/*!
 * Store both type info hashes and string ID hashes.
 * Archive entries are added during static initialization.
 * Therefore we don't protect the access with any locks.
 * Use a 64-bit number and not size_t which may be 32-bits.
 */
static std::map<unsigned long long, Pothos::Archive::ArchiveEntry *> &getArchiveEntryMap(void)
{
    static std::map<unsigned long long, Pothos::Archive::ArchiveEntry *> map;
    return map;
}

/*!
 * Reproducible hash from string to unsigned 64-bit integer.
 */
static unsigned long long hashString64(const std::string &str)
{
    unsigned long long h = 0;
    for (const unsigned char ch : str)
    {
        h = (h * 0xf4243) ^ ch;
    }
    return h;
}

Pothos::Archive::ArchiveEntry::ArchiveEntry(const std::type_info &type, const std::string &id):
    _id(id),
    _hash(hashString64(id))
{
    getArchiveEntryMap()[type.hash_code()] = this;
    getArchiveEntryMap()[_hash] = this;
}

Pothos::Archive::ArchiveEntry::~ArchiveEntry(void)
{
    return;
}

const Pothos::Archive::ArchiveEntry &Pothos::Archive::ArchiveEntry::find(const std::type_info &type)
{
    const auto &map = getArchiveEntryMap();
    const auto it = map.find(type.hash_code());
    if (it != map.end()) return *it->second;
    const auto typeStr = Pothos::Util::typeInfoToString(type);
    throw Pothos::ArchiveException("ArchiveEntry::find("+typeStr+")", "no entry registered for type");
}

const Pothos::Archive::ArchiveEntry &Pothos::Archive::ArchiveEntry::find(const std::string &id)
{
    const auto &map = getArchiveEntryMap();
    const auto it = map.find(hashString64(id));
    if (it != map.end()) return *it->second;
    throw Pothos::ArchiveException("ArchiveEntry::find("+id+")", "no entry registered for GUID");
}

const Pothos::Archive::ArchiveEntry &Pothos::Archive::ArchiveEntry::find(const unsigned long long &hash)
{
    const auto &map = getArchiveEntryMap();
    const auto it = map.find(hash);
    if (it != map.end()) return *it->second;
    throw Pothos::ArchiveException("ArchiveEntry::find("+std::to_string(hash)+")", "no entry registered for hash");
}
