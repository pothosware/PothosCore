///
/// \file System/Info.hpp
///
/// Support for querying information about a host's configuration.
///
/// \copyright
/// Copyright (c) 2013-2014 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <cstddef>
#include <string>

namespace Pothos {
namespace System {

/*!
 * HostInfo contains various OS and network node indentification.
 */
class POTHOS_API HostInfo
{
public:
    /*!
     * Create an empty HostInfo
     */
    HostInfo(void);

    /*!
     * Query the node infomation.
     */
    static HostInfo get(void);

    std::string osName;
    std::string osVersion;
    std::string osArchitecture;
    std::string nodeName;
    std::string nodeId;

    //! The number of CPUs on this system
    size_t processorCount;

    //! The process id of the caller
    std::string pid;
};

} //namespace System
} //namespace Pothos
