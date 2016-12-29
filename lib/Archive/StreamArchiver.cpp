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

Pothos::Archive::IStreamArchiver::IStreamArchiver(std::istream &is, const unsigned int ver):
    is(is), ver(ver)
{
    return;
}

void Pothos::Archive::IStreamArchiver::readBytes(void *buff, const size_t len)
{
    is.read(reinterpret_cast<char *>(buff), len);
}
