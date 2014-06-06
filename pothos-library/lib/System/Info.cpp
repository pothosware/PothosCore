// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Poco/Process.h>
#include <Pothos/System/Info.hpp>
#include <Pothos/Managed.hpp>
#include <Pothos/Object/Serialize.hpp>
#include <Poco/Environment.h>
#include <sigar.h>

//wrapper for dealing with sigar handle
struct SigarHandle
{
    SigarHandle(void)
    {
        sigar_open(&s);
    }
    ~SigarHandle(void)
    {
        sigar_close(s);
    }
    sigar_t *get(void) const
    {
        return s;
    }
    sigar_t *s;
};

/***********************************************************************
 * NodeInfo implementation
 **********************************************************************/
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
    info.pid = std::to_string(Poco::Process::id());
    return info;
}

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
    ar & t.pid;
}
}}

POTHOS_OBJECT_SERIALIZE(Pothos::System::NodeInfo)

/***********************************************************************
 * CpuInfo implementation
 **********************************************************************/
Pothos::System::CpuInfo::CpuInfo(void):
    mhz(0),
    mhzMax(0),
    mhzMin(0),
    cacheSize(0),
    totalSockets(0),
    totalCores(0),
    coresPerSocket(0)
{
    return;
}

std::vector<Pothos::System::CpuInfo> Pothos::System::CpuInfo::get(void)
{
    //query info
    SigarHandle handle;
    sigar_cpu_info_list_t list;
    sigar_cpu_info_list_get(handle.get(), &list);

    //copy into Pothos data structure
    std::vector<CpuInfo> infoList(list.number);
    for (size_t i = 0; i < infoList.size(); i++)
    {
        infoList[i].mhz = list.data[i].mhz;
        infoList[i].mhzMax = list.data[i].mhz_max;
        infoList[i].mhzMin = list.data[i].mhz_min;
        infoList[i].cacheSize = size_t(list.data[i].cache_size);
        infoList[i].totalSockets = list.data[i].total_sockets;
        infoList[i].totalCores = list.data[i].total_cores;
        infoList[i].coresPerSocket = list.data[i].cores_per_socket;
    }

    //free info
    sigar_cpu_info_list_destroy(handle.get(), &list);

    return infoList;
}

static auto managedCpuInfo = Pothos::ManagedClass()
    .registerConstructor<Pothos::System::CpuInfo>()
    .registerStaticMethod(POTHOS_FCN_TUPLE(Pothos::System::CpuInfo, get))
    .commit("Pothos/System/CpuInfo");

namespace Pothos { namespace serialization {
template <class Archive>
void serialize(Archive &ar, Pothos::System::CpuInfo &t, const unsigned int)
{
    ar & t.mhz;
    ar & t.mhzMax;
    ar & t.mhzMin;
    ar & t.cacheSize;
    ar & t.totalSockets;
    ar & t.totalCores;
    ar & t.coresPerSocket;
}
}}

POTHOS_OBJECT_SERIALIZE(Pothos::System::CpuInfo)
POTHOS_OBJECT_SERIALIZE(std::vector<Pothos::System::CpuInfo>)
