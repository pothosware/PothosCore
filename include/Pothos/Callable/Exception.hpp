///
/// \file Callable/Exception.hpp
///
/// Exceptions thrown by the Callable methods.
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
 * A CallableArgumentError is thrown when invalid arguments are used.
 */
POTHOS_DECLARE_EXCEPTION(POTHOS_API, CallableArgumentError, InvalidArgumentException)

/*!
 * A CallableReturnError is thrown when return can't be converted.
 */
POTHOS_DECLARE_EXCEPTION(POTHOS_API, CallableReturnError, NotImplementedException)

/*!
 * A CallableNullError is thrown when a call is made on a null Callable.
 */
POTHOS_DECLARE_EXCEPTION(POTHOS_API, CallableNullError, NullValueException)

} //namespace Pothos
