///
/// \file Proxy/Exception.hpp
///
/// Exceptions thrown by the Proxy methods.
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
 * A ProxyEnvironmentFactoryError is thrown when a factory function fails.
 */
POTHOS_DECLARE_EXCEPTION(POTHOS_API, ProxyEnvironmentFactoryError, RuntimeException);

/*!
 * A ProxyEnvironmentFindError is thrown when a find method fails.
 */
POTHOS_DECLARE_EXCEPTION(POTHOS_API, ProxyEnvironmentFindError, RuntimeException);

/*!
 * A ProxyEnvironmentConvertError is thrown when a convert method fails.
 */
POTHOS_DECLARE_EXCEPTION(POTHOS_API, ProxyEnvironmentConvertError, RuntimeException);

/*!
 * A ProxyHandleCallError is thrown when a call operation fails.
 */
POTHOS_DECLARE_EXCEPTION(POTHOS_API, ProxyHandleCallError, RuntimeException);

/*!
 * A ProxyExceptionMessage is thrown when the call itself has an exception.
 * The exception usually comes from the proxy environment.
 * The message from the exception will be passed through this exception,
 * and retrieved with the what() method.
 */
POTHOS_DECLARE_EXCEPTION(POTHOS_API, ProxyExceptionMessage, RuntimeException);

/*!
 * A ProxyCompareError is thrown when compareTo fails.
 */
POTHOS_DECLARE_EXCEPTION(POTHOS_API, ProxyCompareError, RuntimeException);

/*!
 * A ProxySerializeError is thrown when de/serialize fails.
 */
POTHOS_DECLARE_EXCEPTION(POTHOS_API, ProxySerializeError, RuntimeException);

} //namespace Pothos
