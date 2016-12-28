// Copyright (c) 2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Archive.hpp>
#include <Pothos/Archive/String.hpp>
#include <Pothos/Archive/Numbers.hpp>
#include <iostream>

void Pothos::Archive::OStreamArchiver::writeBytes(const void *buff, const size_t len)
{
    os.write(reinterpret_cast<const char *>(buff), len);
}

void Pothos::Archive::OStreamArchiver::writeInt32(const int num)
{
    char buff[4];
    buff[0] = num >> 0;
    buff[1] = num >> 8;
    buff[2] = num >> 16;
    buff[3] = num >> 24;
    this->writeBytes(buff, sizeof(buff));
}

void Pothos::Archive::OStreamArchiver::writeInt64(const long long num)
{
    char buff[8];
    buff[0] = num >> 0;
    buff[1] = num >> 8;
    buff[2] = num >> 16;
    buff[3] = num >> 24;
    buff[4] = num >> 32;
    buff[5] = num >> 40;
    buff[6] = num >> 48;
    buff[7] = num >> 56;
    this->writeBytes(buff, sizeof(buff));
}

void Pothos::Archive::OStreamArchiver::writeType(const std::type_info &)
{
    //TODO
}

int Pothos::Archive::IStreamArchiver::readInt32(void)
{
    char buff[4];
    this->readBytes(buff, sizeof(buff));
    return
        (static_cast<unsigned int>(buff[0]) << 0) |
        (static_cast<unsigned int>(buff[1]) << 8) |
        (static_cast<unsigned int>(buff[2]) << 16) |
        (static_cast<unsigned int>(buff[3]) << 24);
}

long long Pothos::Archive::IStreamArchiver::readInt64(void)
{
    char buff[8];
    this->readBytes(buff, sizeof(buff));
    return
        (static_cast<unsigned long long>(buff[0]) << 0) |
        (static_cast<unsigned long long>(buff[1]) << 8) |
        (static_cast<unsigned long long>(buff[2]) << 16) |
        (static_cast<unsigned long long>(buff[3]) << 24) |
        (static_cast<unsigned long long>(buff[4]) << 32) |
        (static_cast<unsigned long long>(buff[5]) << 40) |
        (static_cast<unsigned long long>(buff[6]) << 48) |
        (static_cast<unsigned long long>(buff[7]) << 56);
}

void Pothos::Archive::IStreamArchiver::readBytes(void *buff, const size_t len)
{
    is.read(reinterpret_cast<char *>(buff), len);
}

Pothos::Archive::ArchiveEntry::ArchiveEntry(const std::type_info &type, const char *id)
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

POTHOS_CLASS_EXPORT_ID(BooHoo, "BooHoo");

void test(void)
{
    std::stringstream ss;
    BooHoo bh;
    Pothos::Archive::OStreamArchiver ar(ss, 0);
    ar << bh;
    //std::string s("xyz");
    //Pothos::Archive::serializeArchive(ss, s);
}
