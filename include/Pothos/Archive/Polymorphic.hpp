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
#include <Pothos/Archive/Numbers.hpp>
#include <Pothos/Archive/Invoke.hpp>

namespace Pothos {
namespace serialization {

template<typename Archive, typename T>
void save(Archive &ar, T* const &t, const unsigned int)
{
    const auto &entry = Pothos::Archive::ArchiveEntry::find(typeid(*t));
    ar << entry.getHash();
    entry.save(&ar, t);
}

template<typename Archive, typename T>
void load(Archive &ar, T* &t, const unsigned int)
{
    unsigned long long idHash;
    ar >> idHash;
    const auto &entry = Pothos::Archive::ArchiveEntry::find(idHash);
    delete t;
    t = (T*)entry.load(&ar);
}

template <typename Archive, typename T>
void serialize(Archive &ar, T* &t, const unsigned int ver)
{
    Pothos::serialization::invokeSplit(ar, t, ver);
}

}}
