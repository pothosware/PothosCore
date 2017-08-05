// Copyright (c) 2014-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/TopologyImpl.hpp>
#include "Framework/TopologyImpl.hpp"
#include <Pothos/Proxy.hpp>
#include <future>
#include <json.hpp>

using json = nlohmann::json;

/***********************************************************************
 * create JSON stats object
 **********************************************************************/
static json queryWorkStats(const Pothos::Proxy &block)
{
    //try recursive traversal
    try
    {
        return json::parse(block.call<std::string>("queryJSONStats"));
    }
    catch (const std::exception &) {}

    //otherwise, regular block, query stats
    auto actor = block.get("_actor");
    auto workStats = json::parse(actor.call<std::string>("queryWorkStats"));
    json topStats;
    topStats[block.call<std::string>("uid")] = workStats;
    return topStats;
}

std::string Pothos::Topology::queryJSONStats(void)
{
    json stats;

    //query each block's work stats and key it with the UID
    std::vector<std::shared_future<json>> results;
    for (const auto &block : getObjSetFromFlowList(_impl->flows))
    {
        results.push_back(std::async(std::launch::async, queryWorkStats, block));
    }

    //wait on the futures and record to the object
    for (const auto &result : results)
    {
        const auto workStats = result.get();
        for (auto it = workStats.begin(); it != workStats.end(); ++it)
        {
            stats[it.key()] = it.value();
        }
    }

    //use flat topology to get hierarchical block names
    const auto flatTopologyObj = json::parse(this->dumpJSON());
    const auto flatTopologyBlocks = flatTopologyObj["blocks"];
    for (auto it = flatTopologyBlocks.begin(); it != flatTopologyBlocks.end(); ++it)
    {
        if (not stats.count(it.key())) continue;
        stats[it.key()]["blockName"] = it.value()["name"];
    }

    //return the string-formatted result
    return stats.dump(4);
}
