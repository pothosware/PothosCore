// Copyright (c) 2013-2014 Josh Blum
//                    2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include <Poco/Process.h>
#include <Pothos/System/HostInfo.hpp>
#include <Pothos/System/SIMD.hpp>
#include <Pothos/Managed.hpp>
#include <Pothos/Object/Serialize.hpp>
#include <Poco/Environment.h>

Pothos::System::HostInfo::HostInfo(void)
{
    return;
}

Pothos::System::HostInfo Pothos::System::HostInfo::get(void)
{
    HostInfo info;
    info.osName = Poco::Environment::osName();
    info.osVersion = Poco::Environment::osVersion();
    info.osArchitecture = Poco::Environment::osArchitecture();
    info.nodeName = Poco::Environment::nodeName();
    info.nodeId = Poco::Environment::nodeId();
    info.processorCount = Poco::Environment::processorCount();
    info.pid = std::to_string(Poco::Process::id());
    info.availableSIMDFeatures = Pothos::System::getSupportedSIMDFeatureSets();
    return info;
}

#include <Pothos/Managed.hpp>
#include <Pothos/Object/Serialize.hpp>

static auto managedHostInfo = Pothos::ManagedClass()
    .registerConstructor<Pothos::System::HostInfo>()
    .registerStaticMethod(POTHOS_FCN_TUPLE(Pothos::System::HostInfo, get))
    .commit("Pothos/System/HostInfo");

namespace Pothos { namespace serialization {
template <class Archive>
void serialize(Archive &ar, Pothos::System::HostInfo &t, const unsigned int)
{
    ar & t.osName;
    ar & t.osVersion;
    ar & t.osArchitecture;
    ar & t.nodeName;
    ar & t.nodeId;
    ar & t.processorCount;
    ar & t.pid;
    ar & t.availableSIMDFeatures;
}
}}

POTHOS_OBJECT_SERIALIZE(Pothos::System::HostInfo)
