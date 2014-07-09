//
// System/Info.hpp
//
// Information gathering about the system configuration.
//
// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <vector>
#include <string>

namespace Pothos {
namespace System {

/*!
 * CpuInfo is a simple struct loaded with query information.
 */
class POTHOS_API CpuInfo
{
public:
    /*!
     * Creates a blank CpuInfo with values initialized to zero.
     */
    CpuInfo(void);

    /*!
     * Query a list of per-processor CpuInfo structs.
     */
    static std::vector<CpuInfo> get(void);

    std::string vendor;
    std::string model;
    int mhz;
    int mhzMax;
    int mhzMin;
    size_t cacheSize;
    int totalSockets;
    int totalCores;
    int coresPerSocket;
};

} //namespace System
} //namespace Pothos
