///
/// \file System/NumaInfo.hpp
///
/// Support for querying information about NUMA.
///
/// \copyright
/// Copyright (c) 2013-2014 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <vector>
#include <cstddef>

namespace Pothos {
namespace System {

/*!
 * NumaInfo contains information about a NUMA node on the system.
 */
class POTHOS_API NumaInfo
{
public:

    //! Create a blank NumaNodeInfo with empty values
    NumaInfo(void);

    /*!
     * Query a list of per-node NumaInfo structs.
     */
    static std::vector<NumaInfo> get(void);

    //! The numeric identifier for this NUMA node
    size_t nodeNumber;

    //! The total memory on this NUMA node in bytes
    size_t totalMemory;

    //! The available memory on this NUMA node in bytes
    size_t freeMemory;

    //! A list of CPUs located on this NUMA node
    std::vector<size_t> cpus;
};

} //namespace System
} //namespace Pothos
