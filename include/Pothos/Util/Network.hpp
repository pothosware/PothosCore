///
/// \file Util/Network.hpp
///
/// Utility functions to help with networking.
///
/// \copyright
/// Copyright (c) 2015-2015 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <string>

namespace Pothos {
namespace Util {

/*!
 * Does this system support IPv6?
 * \return true if supported
 */
POTHOS_API bool isIPv6Supported(void);

/*!
 * Get a functioning wildcard address "::" or "0.0.0.0".
 * \param service an optional service or port number
 * \return the wildcard address (IPv6 is available)
 */
POTHOS_API std::string getWildcardAddr(const std::string &service = "");

/*!
 * Get a functioning loopback address "::1" or "127.0.0.1".
 * \param service an optional service or port number
 * \return the loopback address (IPv6 is available)
 */
POTHOS_API std::string getLoopbackAddr(const std::string &service = "");

} //namespace Util
} //namespace Pothos
