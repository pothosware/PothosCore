// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/System/NumaInfo.hpp>
#include <Pothos/Managed.hpp>
#include <Pothos/Object/Serialize.hpp>

Pothos::System::NumaInfo::NumaInfo(void):
    nodeNumber(0),
    totalMemory(0),
    freeMemory(0)
{
    return;
}

static auto managedNumaInfo = Pothos::ManagedClass()
    .registerConstructor<Pothos::System::NumaInfo>()
    .registerStaticMethod(POTHOS_FCN_TUPLE(Pothos::System::NumaInfo, get))
    .commit("Pothos/System/NumaInfo");

namespace Pothos { namespace serialization {
template <class Archive>
void serialize(Archive &ar, Pothos::System::NumaInfo &t, const unsigned int)
{
    ar & t.nodeNumber;
    ar & t.totalMemory;
    ar & t.freeMemory;
    ar & t.cpus;
}
}}

POTHOS_OBJECT_SERIALIZE(Pothos::System::NumaInfo)
POTHOS_OBJECT_SERIALIZE(std::vector<Pothos::System::NumaInfo>)
