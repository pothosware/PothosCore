// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include "PothosGui.hpp"
#include <string>
#include <Poco/JSON/Object.h>

//! Get a block description given the block registry path
Poco::JSON::Object::Ptr getBlockDescFromPath(const std::string &path);
