// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QColor>
#include <string>

//! Convert the type name to a displayable color
QColor typeStrToColor(const std::string &typeStr);
