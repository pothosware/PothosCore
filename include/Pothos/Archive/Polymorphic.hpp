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
#include <type_traits>

namespace Pothos {
namespace serialization {

template <typename Archive, typename T>
typename std::enable_if<std::is_pointer<T>::value>::type
save(Archive &ar, const T &t, const unsigned int)
{
    const auto &entry = Pothos::Archive::ArchiveEntry::find(typeid(*t));
    ar << entry.getHash();
    entry.save(ar, t);
}

template <typename Archive, typename T>
typename std::enable_if<std::is_pointer<T>::value>::type
load(Archive &ar, T &t, const unsigned int)
{
    unsigned long long idHash;
    ar >> idHash;
    const auto &entry = Pothos::Archive::ArchiveEntry::find(idHash);
    delete t; //delete previous pointer or its null
    t = static_cast<T>(entry.load(ar));
}

template <typename Archive, typename T>
typename std::enable_if<std::is_pointer<T>::value>::type
serialize(Archive &ar, T &t, const unsigned int ver)
{
    Pothos::serialization::invokeSplit(ar, t, ver);
}

}}
