// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/System/NumaInfo.hpp>
#include <Poco/Environment.h>
#include <sys/types.h>
#include <sys/sysctl.h>

//https://developer.apple.com/library/Mac/releasenotes/Performance/RN-AffinityAPI/index.html
//Note: NUMA INFO Other assumes one NUMA node with all CPUs

//TODO: why not hw.memsize?

inline uint64_t get_hw_cachsize(const int index)
{
    //query the size of the cachesize entries
    size_t size = 0;
    if (sysctlbyname("hw.cachesize", NULL, &size, NULL, 0) != 0) return -1;

    //query the cachesizes
    uint64_t cachesize[size/sizeof(uint64_t)];
    const int ret = sysctlbyname("hw.cachesize", cachesize, &size, NULL, 0);
    if (ret == 0 && size >= sizeof(uint64_t)) return cachesize[index];
    return 0;
}

std::vector<Pothos::System::NumaInfo> Pothos::System::NumaInfo::get(void)
{
    std::vector<NumaInfo> infoList;
    NumaInfo info;

    //a list of all the CPUs
    for (size_t i = 0; i < Poco::Environment::processorCount(); i++)
    {
        info.cpus.push_back(i);
    }

    info.totalMemory = get_hw_cachsize(0); //hw.cachesize[0] reports the size of memory.

    infoList.push_back(info);
    return infoList;
}
