///
/// \file System/Version.hpp
///
/// Version information for the library.
///
/// \copyright
/// Copyright (c) 2013-2014 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <string>

/*!
 * ABI Version Information - incremented when the abi is changed.
 */
#define POTHOS_ABI_VERSION "0.1-0"

namespace Pothos {
namespace System {

    /*!
     * Get the version string - major.minor.path-info
     */
    POTHOS_API std::string getApiVersion(void);

    /*!
     * Get the ABI version string.
     * This is the POTHOS_ABI_VERSION that the library was built against.
     * A client can check POTHOS_ABI_VERSION vs the get_abi()
     * to check for ABI incompatibility before loading the module.
     */
    POTHOS_API std::string getAbiVersion(void);

} //namespace System
} //namespace Pothos
