// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <numa.h>

static Poco::JSON::Object::Ptr enumerateNumaInfo(void)
{
    Poco::JSON::Object::Ptr topObject = new Poco::JSON::Object();
    if (numa_available() < 0) return topObject;

    Poco::JSON::Array::Ptr nodeArray = new Poco::JSON::Array();
    topObject->set("NUMA Node", nodeArray);
    for (int node = 0; node <= numa_max_node(); node++)
    {
        Poco::JSON::Object::Ptr nodeInfo = new Poco::JSON::Object();
        nodeArray->add(nodeInfo);
        long long free = 0; //free bytes
        auto bytes = numa_node_size64(node, &free);
        nodeInfo->set("Memory", std::to_string(bytes/1024/1024)+" MB");
        auto bm = numa_allocate_cpumask();
        if (numa_node_to_cpus(node, bm) == 0)
        {
            std::string out;
            for (size_t i = 0; i < bm->size; i++)
            {
                if (numa_bitmask_isbitset(bm, i))
                {
                    if (not out.empty()) out += ", ";
                    out += std::to_string(i);
                }
            }
            nodeInfo->set("CPUs", out);
        }
        numa_free_nodemask(bm);
    }

    return topObject;
}

pothos_static_block(registerLinuxNumaInfo)
{
    Pothos::PluginRegistry::addCall(
        "/devices/linux_numa/info", &enumerateNumaInfo);
}
