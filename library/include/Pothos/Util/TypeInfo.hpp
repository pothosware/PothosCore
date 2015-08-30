///
/// \file Util/TypeInfo.hpp
///
/// Utility functions dealing with std::type_info
///
/// \copyright
/// Copyright (c) 2013-2014 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <typeinfo>
#include <string>

namespace Pothos {
namespace Util {

/*!
 * Helper method to get the string representation of a type info.
 * The implementation may look up in a table or demangles type.name().
 */
POTHOS_API std::string typeInfoToString(const std::type_info &type);

} //namespace Util
} //namespace Pothos
