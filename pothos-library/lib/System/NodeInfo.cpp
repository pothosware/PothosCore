// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Poco/Process.h>
#include <Pothos/System/NodeInfo.hpp>
#include <Pothos/Managed.hpp>
#include <Pothos/Object/Serialize.hpp>
#include <Poco/Environment.h>

Pothos::System::NodeInfo::NodeInfo(void)
{
    return;
}

Pothos::System::NodeInfo Pothos::System::NodeInfo::get(void)
{
    NodeInfo info;
    info.osName = Poco::Environment::osName();
    info.osVersion = Poco::Environment::osVersion();
    info.osArchitecture = Poco::Environment::osArchitecture();
    info.nodeName = Poco::Environment::nodeName();
    info.nodeId = Poco::Environment::nodeId();
    info.processorCount = Poco::Environment::processorCount();
    info.pid = std::to_string(Poco::Process::id());
    return info;
}

#include <Pothos/Managed.hpp>
#include <Pothos/Object/Serialize.hpp>

static auto managedNodeInfo = Pothos::ManagedClass()
    .registerConstructor<Pothos::System::NodeInfo>()
    .registerStaticMethod(POTHOS_FCN_TUPLE(Pothos::System::NodeInfo, get))
    .commit("Pothos/System/NodeInfo");

namespace Pothos { namespace serialization {
template <class Archive>
void serialize(Archive &ar, Pothos::System::NodeInfo &t, const unsigned int)
{
    ar & t.osName;
    ar & t.osVersion;
    ar & t.osArchitecture;
    ar & t.nodeName;
    ar & t.nodeId;
    ar & t.processorCount;
    ar & t.pid;
}
}}

POTHOS_OBJECT_SERIALIZE(Pothos::System::NodeInfo)
