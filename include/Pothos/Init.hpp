///
/// \file Pothos/Init.hpp
///
/// Initialization calls for library
///
/// \copyright
/// Copyright (c) 2013-2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>

namespace Pothos {

    /*!
     * Perform initialization routines.
     * This call will check the integrity of the installation.
     * This looks for the root directory, the runtime library,
     * permissions on user configuration and data directories.
     * Also, the runtime modules will be loaded into the system.
     * This call will throw if any of the above fails.
     * Subsequent calls to init() are safe and result in NOP.
     */
    POTHOS_API void init(void);

    /*!
     * Perform cleanup routines before application exit.
     * The deinit() call unloads plugin modules loaded by init()
     * Match any calls to init() with to deinit() before application exit.
     */
    POTHOS_API void deinit(void);

    /*!
     * Scoped initializer with automatic cleanup.
     */
    class POTHOS_API ScopedInit
    {
    public:
        //! Call init() on object construction.
        ScopedInit(void);

        //! Call deinit() when going out of scope.
        ~ScopedInit(void);
    };

} //namespace Pothos
