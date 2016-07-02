// Copyright (c) 2014-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/TopologyImpl.hpp>
#include "Framework/TopologyImpl.hpp"
#include <Pothos/Proxy.hpp>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>
#include <sstream>
#include <iostream>
#include <future>
#include <cassert>

/***********************************************************************
 * create JSON stats object
 **********************************************************************/
static Poco::JSON::Object::Ptr queryWorkStats(const Pothos::Proxy &block)
{
    //try recursive traversal
    try
    {
        auto json = block.call<std::string>("queryJSONStats");
        const auto result = Poco::JSON::Parser().parse(json);
        return result.extract<Poco::JSON::Object::Ptr>();
    }
    catch (Pothos::Exception &) {}

    //otherwise, regular block, query stats
    auto actor = block.callProxy("get:_actor");
    auto workStats = actor.call<Poco::JSON::Object::Ptr>("queryWorkStats");
    Poco::JSON::Object::Ptr topStats(new Poco::JSON::Object());
    topStats->set(block.call<std::string>("uid"), workStats);
    return topStats;
}

std::string Pothos::Topology::queryJSONStats(void)
{
    Poco::JSON::Object::Ptr stats(new Poco::JSON::Object());

    //query each block's work stats and key it with the UID
    std::vector<std::shared_future<Poco::JSON::Object::Ptr>> results;
    for (const auto &block : getObjSetFromFlowList(_impl->flows))
    {
        results.push_back(std::async(std::launch::async, queryWorkStats, block));
    }

    //wait on the futures and record to the object
    for (const auto &result : results)
    {
        const auto workStats = result.get();
        std::vector<std::string> names; workStats->getNames(names);
        for (const auto &name : names) stats->set(name, workStats->getObject(name));
    }

    //use flat topology to get hierarchical block names
    const auto result = Poco::JSON::Parser().parse(this->dumpJSON());
    const auto flatTopologyObj = result.extract<Poco::JSON::Object::Ptr>();
    const auto flatTopologyBlocks = flatTopologyObj->getObject("blocks");
    std::vector<std::string> names; flatTopologyBlocks->getNames(names);
    for (const auto &name : names)
    {
        if (not stats->has(name)) continue;
        assert(stats->getObject(name));
        const auto topologObj = flatTopologyBlocks->getObject(name);
        assert(topologObj);
        const auto blockName = topologObj->getValue<std::string>("name");
        stats->getObject(name)->set("blockName", blockName);
    }

    //return the string-formatted result
    std::stringstream ss; stats->stringify(ss, 4);
    return ss.str();
}
