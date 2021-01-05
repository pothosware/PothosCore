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

//! Returns the list of features supported by this machine's processor.
POTHOS_API std::vector<std::string> getSupportedSIMDFeatureSet();

//! Used by Pothos's dynamic SIMD dispatchers to choose the most optimal runnable SIMD function.
POTHOS_API std::string getOptimalSIMDFeatureSetKey(const std::vector<std::string>& featureSetKeys);

} //namespace System
} //namespace Pothos
