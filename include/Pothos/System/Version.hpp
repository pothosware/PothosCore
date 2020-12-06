///
/// \file System/Version.hpp
///
/// Version information for the library.
///
/// \copyright
/// Copyright (c) 2013-2018 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <string>

/*!
 * API version number which can be used as a preprocessor check.
 * The format of the version number is encoded as follows:
 * <b>(major << 24) | (minor << 16) | (16 bit increment)</b>.
 * Where the increment can be used to indicate implementation
 * changes, fixes, or API additions within a minor release series.
 *
 * The macro is typically used in an application as follows:
 * \code
 * #if defined(POTHOS_API_VERSION) && (POTHOS_API_VERSION >= 0x00001234)
 * // Use a newer feature from the Pothos framework API
 * #endif
 * \endcode
 */
#define POTHOS_API_VERSION 0x00070000

/*!
 * ABI Version Information - incremented when the ABI is changed.
 * The ABI version format is <b>major.minor-bump</b>. The <i>major.minor</i>
 * comes from the in-progress library version when the change was made,
 * and <i>bump</i> signifies a change to the ABI during library development.
 * The ABI should remain constant across patch releases of the library.
 */
#define POTHOS_ABI_VERSION "0.7-2"

namespace Pothos {
namespace System {

    /*!
     * Get the Pothos framework API version as a string.
     * The format of the version string is <b>major.minor.increment</b>,
     * where the digits are taken directly from <b>POTHOS_API_VERSION</b>.
     */
    POTHOS_API std::string getApiVersion(void);

    /*!
     * Get the ABI version string that the library was built against.
     * A client can compare <b>POTHOS_ABI_VERSION</b> to getAbiVersion()
     * to check for ABI incompatibility before using the library.
     * If the values are not equal then the client code was
     * compiled against a different ABI than the library.
     */
    POTHOS_API std::string getAbiVersion(void);

    /*!
     * Get the library version and build information string.
     * The format of the version string is <b>major.minor.patch-buildInfo</b>.
     * This function is commonly used to identify the software back-end
     * to the user for command-line utilities and graphical applications.
     */
    POTHOS_API std::string getLibVersion(void);

} //namespace System
} //namespace Pothos
