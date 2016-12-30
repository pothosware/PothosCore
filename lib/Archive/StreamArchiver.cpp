// Copyright (c) 2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Archive/StreamArchiver.hpp>
#include <Pothos/Archive/Numbers.hpp>
#include <iostream>

#define POTHOS_ARCHIVE_VERSION 1

Pothos::Archive::OStreamArchiver::OStreamArchiver(std::ostream &os):
    os(os), ver(POTHOS_ARCHIVE_VERSION)
{
    *this << ver;
}

void Pothos::Archive::OStreamArchiver::writeBytes(const void *buff, const size_t len)
{
    os.write(reinterpret_cast<const char *>(buff), len);
}

Pothos::Archive::IStreamArchiver::IStreamArchiver(std::istream &is):
    is(is), ver(0)
{
    *this >> ver;
}

void Pothos::Archive::IStreamArchiver::readBytes(void *buff, const size_t len)
{
    is.read(reinterpret_cast<char *>(buff), len);
}
