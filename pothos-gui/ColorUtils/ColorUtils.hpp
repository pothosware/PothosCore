// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QColor>
#include <string>
#include <map>

//! Convert the type name to a displayable color
QColor typeStrToColor(const std::string &typeStr);

//! Get the map of known type strings to colors
std::map<std::string, QColor> getTypeStrToColorMap(void);
