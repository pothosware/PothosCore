// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once

#include <Pothos/Config.hpp>
#include <qglobal.h>

#if defined(POTHOS_WIDGET_UTILS_MAKEDLL)     // create a DLL library 
#define POTHOS_WIDGET_UTILS_EXPORT Q_DECL_EXPORT
#else                        // use a DLL library
#define POTHOS_WIDGET_UTILS_EXPORT Q_DECL_IMPORT 
#endif
