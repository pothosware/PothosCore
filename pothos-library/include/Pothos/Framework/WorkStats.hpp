///
/// \file Framework/WorkStats.hpp
///
/// This file contains the interface for work statistics.
///
/// \copyright
/// Copyright (c) 2014-2015 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <chrono>

namespace Pothos {

/*!
 * Statistics about an individual port.
 */
struct POTHOS_API PortStats
{
    PortStats(void);
    std::chrono::high_resolution_clock::time_point timeLastBuffer;
    std::chrono::high_resolution_clock::time_point timeLastMessage;
    std::chrono::high_resolution_clock::time_point timeLastLabel;
    unsigned long long totalElements;
    unsigned long long totalMessages;
    unsigned long long totalLabels;
};

/*!
 * Statistics about a block's processing function.
 */
struct POTHOS_API WorkStats
{
    WorkStats(void);
    std::chrono::high_resolution_clock::duration totalTimeWork;
    std::chrono::high_resolution_clock::duration totalTimePreWork;
    std::chrono::high_resolution_clock::duration totalTimePostWork;
    unsigned long long numWorkCalls;
    unsigned long long bytesConsumed;
    unsigned long long bytesProduced;
    unsigned long long msgsConsumed;
    unsigned long long msgsProduced;
    std::chrono::high_resolution_clock::time_point timeLastConsumed;
    std::chrono::high_resolution_clock::time_point timeLastProduced;
    std::chrono::high_resolution_clock::time_point timeLastWork;
    std::chrono::high_resolution_clock::time_point timeStatsQuery;
};

//! Helper routine to deal with automatically accumulating time durations
struct TimeAccumulator
{
    inline TimeAccumulator(std::chrono::high_resolution_clock::duration &t):
        t(t), start(std::chrono::high_resolution_clock::now())
    {
        return;
    }
    inline ~TimeAccumulator(void)
    {
        t += std::chrono::high_resolution_clock::now() - start;
    }
    std::chrono::high_resolution_clock::duration &t;
    const std::chrono::high_resolution_clock::time_point start;
};

} //namespace Pothos
