// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <chrono>
#include <Theron/Detail/Threading/Clock.h>

/*
struct PortStats
{
    PortStats(void);
    unsigned long long timeLastBuffer;
    unsigned long long timeLastMessage;
    unsigned long long timeLastLabel;
    unsigned long long totalElements;
    unsigned long long totalMessages;
    unsigned long long totalLabels;
};
*/

struct WorkerStats
{
    WorkerStats(void);
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
