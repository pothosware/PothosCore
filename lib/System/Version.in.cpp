// Copyright (c) 2013-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/System/Version.hpp>
#include <Poco/Format.h>

std::string Pothos::System::getApiVersion(void)
{
    return Poco::format("%d.%d.%d",
        int((POTHOS_API_VERSION >> 24) & 0xf),
        int((POTHOS_API_VERSION >> 16) & 0xf),
        int((POTHOS_API_VERSION >> 0) & 0xff));
}

std::string Pothos::System::getAbiVersion(void)
{
    return POTHOS_ABI_VERSION;
}

std::string Pothos::System::getLibVersion(void)
{
    return "@POTHOS_VERSION@";
}
