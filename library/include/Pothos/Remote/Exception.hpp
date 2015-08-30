///
/// \file Remote/Exception.hpp
///
/// Exceptions thrown by the Remote methods.
///
/// \copyright
/// Copyright (c) 2013-2014 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Exception.hpp>

namespace Pothos {

/*!
 * A RemoteClientError is thrown when a the server connect fails.
 */
POTHOS_DECLARE_EXCEPTION(POTHOS_API, RemoteClientError, RuntimeException)

/*!
 * A RemoteServerError is thrown when a the server spawn fails.
 */
POTHOS_DECLARE_EXCEPTION(POTHOS_API, RemoteServerError, RuntimeException)

} //namespace Pothos
