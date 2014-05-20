//
// System/Paths.hpp
//
// System paths for installing and locating system files.
//
// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <string>

namespace Pothos {
namespace System {

    /*!
     * Get the root path of the Pothos installation.
     * The root path is set by the POTHOS_ROOT environment variable.
     * Otherwise set by the CMAKE_INSTALL_PREFIX at configuration time.
     */
    POTHOS_API std::string getRootPath(void);

    /*!
     * Get the data path of the Pothos installation.
     * This should be getRootPath()/share/Pothos
     */
    POTHOS_API std::string getDataPath(void);

    /*!
     * Where to put user local data.
     * UNIX: This is $XDG_DATA_HOME or $HOME/.local/share/Pothos
     * Windows: This is %APPDATA%/Pothos
     */
    POTHOS_API std::string getUserDataPath(void);

    /*!
     * Where to put user local config.
     * UNIX: This is $XDG_CONFIG_HOME or $HOME/.config/Pothos
     * Windows: This is %APPDATA%/Pothos
     */
    POTHOS_API std::string getUserConfigPath(void);

    /*!
     * Get the complete path for the PothosUtil executable.
     * The executable should be found in getRootPath()/bin.
     */
    POTHOS_API std::string getPothosUtilExecutablePath(void);

    /*!
     * Get the complete path to the Pothos runtime library.
     */
    POTHOS_API std::string getPothosRuntimeLibraryPath(void);

    /*!
     * Get the full path to the development headers directory.
     */
    POTHOS_API std::string getPothosDevIncludePath(void);

    /*!
     * Get the full path to the development libraries directory.
     */
    POTHOS_API std::string getPothosDevLibraryPath(void);

} //namespace System
} //namespace Pothos
