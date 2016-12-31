///
/// \file Archive/Exception.hpp
///
/// Exceptions thrown by the Archive methods.
///
/// \copyright
/// Copyright (c) 2016-2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Exception.hpp>

namespace Pothos {

/*!
 * An ArchiveException is thrown when an archive operation fails.
 */
POTHOS_DECLARE_EXCEPTION(POTHOS_API, ArchiveException, RuntimeException);

} //namespace Pothos
