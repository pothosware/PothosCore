//
// Framework/Exception.hpp
//
// Exceptions thrown by the Framework methods.
//
// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Exception.hpp>

namespace Pothos {

/*!
 * A SharedBufferError is thrown when the SharedBuffer cant allocate.
 */
POTHOS_DECLARE_EXCEPTION(POTHOS_API, SharedBufferError, RuntimeException)

/*!
 * A BufferManagerFactoryError is thrown when the factory has an error.
 */
POTHOS_DECLARE_EXCEPTION(POTHOS_API, BufferManagerFactoryError, RuntimeException)

/*!
 * A BufferPushError is thrown when buffers are pushed to the wrong queue.
 */
POTHOS_DECLARE_EXCEPTION(POTHOS_API, BufferPushError, RuntimeException)

/*!
 * A PortAccessError is thrown when an invalid worker port identifier is used.
 */
POTHOS_DECLARE_EXCEPTION(POTHOS_API, PortAccessError, RangeException)

/*!
 * A DTypeUnknownError is thrown when an unknown DType identifier is used.
 */
POTHOS_DECLARE_EXCEPTION(POTHOS_API, DTypeUnknownError, RuntimeException)

/*!
 * A TopologyConnectError is thrown when a dis/connection is not possible.
 */
POTHOS_DECLARE_EXCEPTION(POTHOS_API, TopologyConnectError, RuntimeException)

/*!
 * A BlockCallNotFound is thrown when a call by the given name cannot be found.
 */
POTHOS_DECLARE_EXCEPTION(POTHOS_API, BlockCallNotFound, RuntimeException)

/*!
 * A PortDomainError is thrown from getInput/OutputBufferManager() for incompatible domains.
 */
POTHOS_DECLARE_EXCEPTION(POTHOS_API, PortDomainError, RuntimeException)

/*!
 * A ThreadPoolError is thrown when ThreadPoolArgs are invalid.
 */
POTHOS_DECLARE_EXCEPTION(POTHOS_API, ThreadPoolError, RuntimeException)

} //namespace Pothos
