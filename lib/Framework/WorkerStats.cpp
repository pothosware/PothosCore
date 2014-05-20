// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/WorkerStats.hpp"

WorkerStats::WorkerStats(void):
    tickRate(Theron::Detail::Clock::GetFrequency()),
    totalTicksWork(0),
    totalTicksPreWork(0),
    totalTicksPostWork(0),
    numWorkCalls(0),
    bytesConsumed(0),
    bytesProduced(0),
    msgsConsumed(0),
    msgsProduced(0),
    ticksLastConsumed(0),
    ticksLastProduced(0),
    ticksLastWork(0),
    ticksStatsQuery(0)
{
    return;
}

#include <Pothos/Object/Serialize.hpp>

namespace Pothos { namespace serialization {
template <class Archive>
void serialize(Archive &ar, WorkerStats &t, const unsigned int)
{
    ar & t.totalTicksWork;
    ar & t.totalTicksPreWork;
    ar & t.totalTicksPostWork;
    ar & t.numWorkCalls;
    ar & t.bytesConsumed;
    ar & t.bytesProduced;
    ar & t.msgsConsumed;
    ar & t.msgsProduced;
    ar & t.ticksLastConsumed;
    ar & t.ticksLastProduced;
    ar & t.ticksLastWork;
    ar & t.ticksStatsQuery;
}
}}

POTHOS_OBJECT_SERIALIZE(WorkerStats)
