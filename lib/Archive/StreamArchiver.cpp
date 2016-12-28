// Copyright (c) 2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Archive/StreamArchiver.hpp>
#include <iostream>

Pothos::Archive::OStreamArchiver::OStreamArchiver(std::ostream &os, const unsigned int ver):
    os(os), ver(ver)
{
    return;
}

void Pothos::Archive::OStreamArchiver::writeBytes(const void *buff, const size_t len)
{
    os.write(reinterpret_cast<const char *>(buff), len);
}

void Pothos::Archive::OStreamArchiver::writeInt32(const unsigned int num)
{
    unsigned char buff[4];
    buff[0] = char(num >> 0);
    buff[1] = char(num >> 8);
    buff[2] = char(num >> 16);
    buff[3] = char(num >> 24);
    this->writeBytes(buff, sizeof(buff));
}

void Pothos::Archive::OStreamArchiver::writeInt64(const unsigned long long num)
{
    unsigned char buff[8];
    buff[0] = char(num >> 0);
    buff[1] = char(num >> 8);
    buff[2] = char(num >> 16);
    buff[3] = char(num >> 24);
    buff[4] = char(num >> 32);
    buff[5] = char(num >> 40);
    buff[6] = char(num >> 48);
    buff[7] = char(num >> 56);
    this->writeBytes(buff, sizeof(buff));
}

void Pothos::Archive::OStreamArchiver::writeType(const std::type_info &)
{
    //TODO
}

Pothos::Archive::IStreamArchiver::IStreamArchiver(std::istream &is, const unsigned int ver):
    is(is), ver(ver)
{
    return;
}

unsigned int Pothos::Archive::IStreamArchiver::readInt32(void)
{
    unsigned char buff[4];
    this->readBytes(buff, sizeof(buff));
    return
        (static_cast<unsigned int>(buff[0]) << 0) |
        (static_cast<unsigned int>(buff[1]) << 8) |
        (static_cast<unsigned int>(buff[2]) << 16) |
        (static_cast<unsigned int>(buff[3]) << 24);
}

unsigned long long Pothos::Archive::IStreamArchiver::readInt64(void)
{
    unsigned char buff[8];
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
