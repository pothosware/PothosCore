// Copyright (c) 2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Archive/ArchiveEntry.hpp>
#include <Pothos/Archive/Exception.hpp>
#include <Pothos/Util/TypeInfo.hpp>
#include <iostream>
#include <map>

static std::map<size_t, Pothos::Archive::ArchiveEntry *> &getArchiveEntryTypeMap(void)
{
    static std::map<size_t, Pothos::Archive::ArchiveEntry *> map;
    return map;
}

static std::map<std::string, Pothos::Archive::ArchiveEntry *> &getArchiveEntryIdMap(void)
{
    static std::map<std::string, Pothos::Archive::ArchiveEntry *> map;
    return map;
}

Pothos::Archive::ArchiveEntry::ArchiveEntry(const std::type_info &type, const std::string &id):
    _id(id)
{
    getArchiveEntryTypeMap()[type.hash_code()] = this;
    getArchiveEntryIdMap()[id] = this;
}

const Pothos::Archive::ArchiveEntry &Pothos::Archive::ArchiveEntry::getEntryFromType(const std::type_info &type)
{
    const auto &map = getArchiveEntryTypeMap();
    const auto it = map.find(type.hash_code());
    if (it != map.end()) return *it->second;
    const auto typeStr = Pothos::Util::typeInfoToString(type);
    throw Pothos::ArchiveException("ArchiveEntry::getEntryFromType("+typeStr+")", "no entry registered for type");
}

const Pothos::Archive::ArchiveEntry &Pothos::Archive::ArchiveEntry::getEntryFromGUID(const std::string &id)
{
    const auto &map = getArchiveEntryIdMap();
    const auto it = map.find(id);
    if (it != map.end()) return *it->second;
    throw Pothos::ArchiveException("ArchiveEntry::getEntryFromGUID("+id+")", "no entry registered for id");
}
