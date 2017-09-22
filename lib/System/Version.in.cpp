// Copyright (c) 2013-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/System/Version.hpp>
#include <sstream>

std::string Pothos::System::getApiVersion(void)
{
    std::stringstream ss;
    ss << std::hex << int((POTHOS_API_VERSION >> 24) & 0xff) << "."
       << std::hex << int((POTHOS_API_VERSION >> 16) & 0xff) << "."
       << std::hex << int((POTHOS_API_VERSION >> 0) & 0xffff)
       << std::dec;
    return ss.str();
}

std::string Pothos::System::getAbiVersion(void)
{
    return POTHOS_ABI_VERSION;
}

std::string Pothos::System::getLibVersion(void)
{
    return "@POTHOS_VERSION@";
}
