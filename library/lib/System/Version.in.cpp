// Copyright (c) 2013 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/System/Version.hpp>

std::string Pothos::System::getApiVersion(void)
{
    return "@POTHOS_VERSION@";
}

std::string Pothos::System::getAbiVersion(void)
{
    return POTHOS_ABI_VERSION;
}
