// Copyright (c) 2020 Nicholsa Corgan
// SPDX-License-Identifier: BSL-1.0

#include "PothosUtil.hpp"

#include <Pothos/System.hpp>

#include <iostream>

// TODO: change to supported feature sets
void PothosUtilBase::printSIMDArchitectures(void)
{
    for(const auto& arch: Pothos::System::getSupportedSIMDFeatureSet())
    {
        std::cout << arch << std::endl;
    }
}
