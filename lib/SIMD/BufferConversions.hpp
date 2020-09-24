// Copyright (c) 2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#pragma once

#include <vector>

namespace PothosSIMD
{    
    template <typename InType, typename OutType>
    void bufferConvert(const void* in, void* out, size_t len);
    
    template <typename InType, typename OutType>
    std::vector<OutType> vectorConvert(const std::vector<InType>& in);
}