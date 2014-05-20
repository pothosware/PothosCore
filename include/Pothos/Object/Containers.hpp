//
// Object/Containers.hpp
//
// Common typedefs for stl containers of type Object.
//
// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Object.hpp>
#include <string>
#include <vector>
#include <set>
#include <map>

namespace Pothos {

typedef std::vector<Object> ObjectVector;

typedef std::set<Object> ObjectSet;

typedef std::map<Object, Object> ObjectMap;

typedef std::map<std::string, Object> ObjectKwargs;

} // namespace Pothos
