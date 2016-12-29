///
/// \file Archive/Polymorphic.hpp
///
/// Polymorphic pointer support for serialization.
///
/// \copyright
/// Copyright (c) 2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Archive/ArchiveEntry.hpp>
#include <Pothos/Archive/String.hpp>
#include <Pothos/Archive/Invoke.hpp>

namespace Pothos {
namespace serialization {

template<typename Archive, typename T>
void save(Archive &ar, T* const& t, const unsigned int)
{
    const auto &entry = Pothos::Archive::ArchiveEntry::getEntryFromType(typeid(*t));
    ar << entry.getId();
    entry.save(&ar, t);
}

template<typename Archive, typename T>
void load(Archive &ar, T* &t, const unsigned int)
{
    std::string id;
    ar >> id;
    const auto &entry = Pothos::Archive::ArchiveEntry::getEntryFromGUID(id);
    delete t;
    t = (T*)entry.load(&ar);
}

template <typename Archive, typename T>
void serialize(Archive &ar, T* &t, const unsigned int ver)
{
    Pothos::serialization::invokeSplit(ar, t, ver);
}

}}
