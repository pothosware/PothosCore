// Copyright (c) 2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Archive.hpp>
#include <iostream>

Pothos::Archive::ArchiveEntry::ArchiveEntry(const std::type_info &type, const char *id)
{
    std::cout << "REGISTER " << type.name() << ", id = " << id << std::endl;
}
