// Copyright (c) 2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Util/Archive.hpp>
#include <iostream>

Pothos::Util::ArchiveEntry::ArchiveEntry(const std::type_info &type, const char *id)
{
    std::cout << "REGISTER " << type.name() << ", id = " << id << std::endl;
}

#include <sstream>

class BooHoo
{
public:
    BooHoo()
    {

    }

int foo;
std::string bar;
};

namespace Pothos {
namespace serialization {

template<class Archive>
void serialize(Archive &a, BooHoo &t, const unsigned int)
{
    a & t.foo;
    a & t.bar;
}

}}

POTHOS_CLASS_EXPORT_GUID(BooHoo, "BooHoo");

void test(void)
{
    BooHoo bh;
    std::stringstream ss;
    Pothos::Util::SerializeArchive(ss, bh);
}
