// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once

#include <Pothos/Config.hpp>
#include <qglobal.h>

#if defined(POTHOS_PLOTTER_UTILS_MAKEDLL)     // create a DLL library 
#define POTHOS_PLOTTER_UTILS_EXPORT Q_DECL_EXPORT
#else                        // use a DLL library
#define POTHOS_PLOTTER_UTILS_EXPORT Q_DECL_IMPORT 
#endif
