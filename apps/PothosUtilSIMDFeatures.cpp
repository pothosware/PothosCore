// Copyright (c) 2020 Nicholsa Corgan
// SPDX-License-Identifier: BSL-1.0

#include "PothosUtil.hpp"

#include <Pothos/System.hpp>

#include <iostream>

void PothosUtilBase::printSIMDFeatures(void)
{
    std::cout << "SIMD features supported on this processor:" << std::endl;
    for(const auto& arch: Pothos::System::getSupportedSIMDFeatureSet())
    {
        std::cout << " * " << arch << std::endl;
    }
}
