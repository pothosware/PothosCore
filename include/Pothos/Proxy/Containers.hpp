//
// Proxy/Containers.hpp
//
// Common typedefs for stl containers of type Proxy.
//
// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Proxy.hpp>
#include <vector>
#include <set>
#include <map>

namespace Pothos {

typedef std::vector<Proxy> ProxyVector;

typedef std::set<Proxy> ProxySet;

typedef std::map<Proxy, Proxy> ProxyMap;

} // namespace Pothos
