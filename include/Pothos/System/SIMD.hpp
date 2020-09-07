///
/// \file System/SIMD.hpp
///
/// Support for querying information about a host's SIMD support.
///
/// \copyright
/// Copyright (c) 2020 Nicholas Corgan
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once

#include <Pothos/Config.hpp>

#include <string>
#include <vector>

namespace Pothos {
namespace System {

POTHOS_API std::vector<std::string> getSupportedSIMDFeatureSets();

POTHOS_API std::string getSIMDFeatureSetKey(const std::vector<std::string>& featureSets);

POTHOS_API bool isSIMDFeatureSetSupported(const std::string& featureSetKey);

} //namespace System
} //namespace Pothos
