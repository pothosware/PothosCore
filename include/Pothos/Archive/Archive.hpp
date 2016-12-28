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
#include <type_traits>
#include <typeinfo>
#include <iosfwd>

namespace Pothos {
namespace Archive {

template <typename T>
struct ArchiveEntryContainer;

class POTHOS_API ArchiveEntry
{
public:

    ArchiveEntry(const std::type_info &type, const char *id);
};

} //namespace Archive
} //namespace Pothos
