// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/WorkStats.hpp>

Pothos::PortStats::PortStats(void):
    totalElements(0),
    totalMessages(0),
    totalLabels(0)
{
    return;
}

Pothos::WorkStats::WorkStats(void):
    totalTimeWork(0),
    totalTimePreWork(0),
    totalTimePostWork(0),
    numWorkCalls(0),
    bytesConsumed(0),
    bytesProduced(0),
    msgsConsumed(0),
    msgsProduced(0)
{
    return;
}

#include <Pothos/Object/Serialize.hpp>

namespace Pothos { namespace serialization {
template<class Archive>
void save(Archive & ar, const std::chrono::high_resolution_clock::duration &t, const unsigned int)
{
    const long long count = t.count();
    ar << count;
}

template<class Archive>
void load(Archive & ar, std::chrono::high_resolution_clock::duration &t, const unsigned int)
{
    long long count = 0;
    ar >> count;
    t = std::chrono::high_resolution_clock::duration(count);
}

template<class Archive>
void save(Archive & ar, const std::chrono::high_resolution_clock::time_point &t, const unsigned int v)
{
    save(ar, t.time_since_epoch(), v);
}

template<class Archive>
void load(Archive & ar, std::chrono::high_resolution_clock::time_point &t, const unsigned int v)
{
    std::chrono::high_resolution_clock::duration dur;
    load(ar, dur, v);
    t = std::chrono::high_resolution_clock::time_point(dur);
}
}}

POTHOS_SERIALIZATION_SPLIT_FREE(std::chrono::high_resolution_clock::duration)
POTHOS_SERIALIZATION_SPLIT_FREE(std::chrono::high_resolution_clock::time_point)

namespace Pothos { namespace serialization {
template <class Archive>
void serialize(Archive &ar, Pothos::PortStats &t, const unsigned int)
{
    ar & t.timeLastBuffer;
    ar & t.timeLastMessage;
    ar & t.timeLastLabel;
    ar & t.totalElements;
    ar & t.totalMessages;
    ar & t.totalLabels;
}
}}

POTHOS_OBJECT_SERIALIZE(Pothos::PortStats)

namespace Pothos { namespace serialization {
template <class Archive>
void serialize(Archive &ar, Pothos::WorkStats &t, const unsigned int)
{
    ar & t.totalTimeWork;
    ar & t.totalTimePreWork;
    ar & t.totalTimePostWork;
    ar & t.numWorkCalls;
    ar & t.bytesConsumed;
    ar & t.bytesProduced;
    ar & t.msgsConsumed;
    ar & t.msgsProduced;
    ar & t.timeLastConsumed;
    ar & t.timeLastProduced;
    ar & t.timeLastWork;
    ar & t.timeStatsQuery;
}
}}

POTHOS_OBJECT_SERIALIZE(Pothos::WorkStats)
