///
/// \file Managed/Exception.hpp
///
/// Exceptions thrown by the Managed methods.
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
 * A ManagedClassLookupError is thrown when the lookup call fails.
 */
POTHOS_DECLARE_EXCEPTION(POTHOS_API, ManagedClassLookupError, RuntimeException);

/*!
 * A ManagedClassTypeError is thrown when a registration type mismatches fails.
 */
POTHOS_DECLARE_EXCEPTION(POTHOS_API, ManagedClassTypeError, RuntimeException);

/*!
 * A ManagedClassNameError is thrown when a method name does not exist.
 */
POTHOS_DECLARE_EXCEPTION(POTHOS_API, ManagedClassNameError, RuntimeException);

} //namespace Pothos
