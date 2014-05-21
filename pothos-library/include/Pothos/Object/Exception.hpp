//
// Object/Exception.hpp
//
// Exceptions thrown by the Object methods.
//
// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Exception.hpp>

namespace Pothos {

/*!
 * An ObjectConvertError is thrown when an object is converted to the wrong type.
 */
POTHOS_DECLARE_EXCEPTION(POTHOS_API, ObjectConvertError, RuntimeException);

/*!
 * An ObjectSerializeError is thrown when an object cannot be de/serialized.
 */
POTHOS_DECLARE_EXCEPTION(POTHOS_API, ObjectSerializeError, RuntimeException)

/*!
 * An ObjectCompareError is thrown when the compareTo() call fails.
 */
POTHOS_DECLARE_EXCEPTION(POTHOS_API, ObjectCompareError, RuntimeException)

} //namespace Pothos
