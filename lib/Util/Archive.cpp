// Copyright (c) 2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Util/Archive.hpp>
#include <iostream>

Pothos::Util::ArchiveEntry::ArchiveEntry(const std::type_info &type, const char *id)
{
    std::cout << "REGISTER " << type.name() << ", id = " << id << std::endl;
}

class BooHoo
{
public:
    BooHoo()
    {
        
    }
};

POTHOS_CLASS_EXPORT_GUID(BooHoo, "BooHoo");
