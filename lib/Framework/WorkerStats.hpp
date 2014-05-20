// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <Theron/Detail/Threading/Clock.h>

/*
struct PortStats
{
    PortStats(void);
    unsigned long long ticksLastBuffer;
    unsigned long long ticksLastMessage;
    unsigned long long ticksLastLabel;
    unsigned long long totalElements;
    unsigned long long totalMessages;
    unsigned long long totalLabels;
};
*/

struct WorkerStats
{
    WorkerStats(void);
    unsigned long long tickRate;
    unsigned long long totalTicksWork;
    unsigned long long totalTicksPreWork;
    unsigned long long totalTicksPostWork;
    unsigned long long numWorkCalls;
    unsigned long long bytesConsumed;
    unsigned long long bytesProduced;
    unsigned long long msgsConsumed;
    unsigned long long msgsProduced;
    unsigned long long ticksLastConsumed;
    unsigned long long ticksLastProduced;
    unsigned long long ticksLastWork;
    unsigned long long ticksStatsQuery;
};

struct TicksAccumulator
{
    inline TicksAccumulator(unsigned long long &t):
        t(t), start(Theron::Detail::Clock::GetTicks())
    {
        return;
    }
    inline ~TicksAccumulator(void)
    {
        t += Theron::Detail::Clock::GetTicks() - start;
    }
    unsigned long long &t;
    unsigned long long start;
};
