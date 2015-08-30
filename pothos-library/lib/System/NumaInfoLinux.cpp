// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/System/NumaInfo.hpp>
#include <numa.h>

std::vector<Pothos::System::NumaInfo> Pothos::System::NumaInfo::get(void)
{
    std::vector<NumaInfo> infoList;
    if (numa_available() < 0) return infoList;
    for (int node = 0; node <= numa_max_node(); node++)
    {
        NumaInfo info;

        info.nodeNumber = node;

        long long free = 0; //free bytes
        info.totalMemory = numa_node_size64(node, &free);
        info.freeMemory = free;

        auto bm = numa_allocate_cpumask();
        if (numa_node_to_cpus(node, bm) == 0)
        {
            for (size_t i = 0; i < bm->size; i++)
            {
                if (numa_bitmask_isbitset(bm, i))
                {
                    info.cpus.push_back(i);
                }
            }
        }
        numa_free_nodemask(bm);
        infoList.push_back(info);
    }
    return infoList;
}
