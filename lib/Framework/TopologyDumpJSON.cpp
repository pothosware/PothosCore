// Copyright (c) 2015-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/TopologyImpl.hpp>
#include "Framework/TopologyImpl.hpp"
#include <Pothos/Proxy.hpp>
#include <json.hpp>

using json = nlohmann::json;

static bool blockIsHier(const json &blockObj)
{
    return blockObj.count("connections") != 0;
}

static std::vector<std::pair<std::string, std::string>> resolvePorts(
    const json &topObj,
    const std::string &blockId,
    const std::string &portName,
    const bool resolveSrc
)
{
    std::vector<std::pair<std::string, std::string>> results;

    const auto blocksObj = topObj["blocks"];
    if (blocksObj.count(blockId) == 0) return results;
    const auto &blockObj = blocksObj[blockId];

    if (not blockIsHier(blockObj))
    {
        results.emplace_back(blockId, portName);
        return results;
    }

    for (const auto &subConnObj : blockObj.value("connections", json::array()))
    {
        std::string subId, subName;

        if (resolveSrc)
        {
            if (subConnObj["dstId"].get<std::string>() != blockId) continue;
            if (subConnObj["dstName"].get<std::string>() != portName) continue;
            subId = subConnObj["srcId"];
            subName = subConnObj["srcName"];
        }
        else
        {
            if (subConnObj["srcId"].get<std::string>() != blockId) continue;
            if (subConnObj["srcName"].get<std::string>() != portName) continue;
            subId = subConnObj["dstId"];
            subName = subConnObj["dstName"];
        }

        //ignore pass-through connections in this loop
        if (subId != blockId) results.emplace_back(subId, subName);
    }

    return results;
}

static bool flattenDump(json &topObj)
{
    bool hierFound = false;

    //create new blocks object that flattens any hierarchy to 1 depth
    //if this block is a hierarchy -- bring its blocks to the top level
    auto &blocksObj = topObj["blocks"];
    json flatBlocksObj(json::object());
    for (auto it = blocksObj.begin(); it != blocksObj.end(); ++it)
    {
        auto &blockObj = it.value();
        if (blockIsHier(blockObj))
        {
            hierFound = true;
            auto &subBlocksObj = blockObj["blocks"];
            const std::string thisName = blockObj["name"];
            for (auto subIt = subBlocksObj.begin(); subIt != subBlocksObj.end(); ++subIt)
            {
                auto &subBlockObj = subIt.value();
                const std::string subName = subBlockObj["name"];
                subBlockObj["name"] = thisName+"/"+subName; //heritage name
                flatBlocksObj[subIt.key()] = subBlockObj;
            }
        }
        else flatBlocksObj[it.key()] = blockObj;
    }

    //create new connections array folding out depth 1 hierarchies
    json flatConnsArray(json::array());
    for (const auto &connObj : topObj.value("connections", json::array()))
    {
        for (const auto & resolvedSrc : resolvePorts(topObj, connObj["srcId"].get<std::string>(), connObj["srcName"].get<std::string>(), true))
        {
            for (const auto & resolvedDst : resolvePorts(topObj, connObj["dstId"].get<std::string>(), connObj["dstName"].get<std::string>(), false))
            {
                json flatConnObj;
                flatConnObj["srcId"] = resolvedSrc.first;
                flatConnObj["srcName"] = resolvedSrc.second;
                flatConnObj["dstId"] = resolvedDst.first;
                flatConnObj["dstName"] = resolvedDst.second;
                flatConnsArray.push_back(flatConnObj);
            }
        }
    }

    //resolve pass-through connections and totally internal connections
    for (auto it = blocksObj.begin(); it != blocksObj.end(); ++it)
    {
        const auto &blockObj = it.value();
        if (not blockIsHier(blockObj)) continue;

        for (const auto &subConnObj : blockObj.value("connections", json::array()))
        {
            const bool srcIsThis = subConnObj["srcId"].get<std::string>() == it.key();
            const bool dstIsThis = subConnObj["dstId"].get<std::string>() == it.key();

            //totally internal connection
            if (not srcIsThis and not dstIsThis) flatConnsArray.push_back(subConnObj);

            //otherwise not a pass-through
            if (not srcIsThis or not dstIsThis) continue;

            //find sources where the destination is this pass-through
            for (const auto &connObj_s : topObj.value("connections", json::array()))
            {
                if (connObj_s["dstId"].get<std::string>() != it.key()) continue;
                if (connObj_s["dstName"].get<std::string>() != subConnObj["srcName"].get<std::string>()) continue;

                //find sources where the destination is this pass-through
                for (const auto &connObj_d : topObj.value("connections", json::array()))
                {
                    if (connObj_d["srcId"].get<std::string>() != it.key()) continue;
                    if (connObj_d["srcName"].get<std::string>() != subConnObj["dstName"].get<std::string>()) continue;

                    json flatConnObj;
                    flatConnObj["srcId"] = connObj_s["srcId"];
                    flatConnObj["srcName"] = connObj_s["srcName"];
                    flatConnObj["dstId"] = connObj_d["dstId"];
                    flatConnObj["dstName"] = connObj_d["dstName"];
                    flatConnsArray.push_back(flatConnObj);
                }
            }
        }
    }

    //set new flat data into the top object
    topObj = json::object();
    topObj["blocks"] = flatBlocksObj;
    topObj["connections"] = flatConnsArray;
    return hierFound;
}

static json portInfoToObj(const Pothos::PortInfo &portInfo)
{
    json infoObj;
    infoObj["name"] = portInfo.name;
    infoObj["alias"] = portInfo.alias;
    infoObj["dtype"] = portInfo.dtype.toMarkup();
    infoObj["size"] = portInfo.dtype.size();
    infoObj["isSigSlot"] = portInfo.isSigSlot;
    return infoObj;
}

std::string Pothos::Topology::dumpJSON(const std::string &request)
{
    //extract input request
    auto configObj = json::parse(request.empty()?"{}":request);
    const std::string modeConfig = configObj.value("mode", "flat");

    //parse request into traversal arguments
    const bool flatten = (modeConfig == "flat");
    const bool traverse = (modeConfig != "rendered");
    const auto &flows = (modeConfig == "rendered")?_impl->activeFlatFlows:_impl->flows;

    //replace rendered names with names from flattened hierarchy
    json flatBlocks(json::object());
    if (modeConfig == "rendered")
    {
        auto flatObj = json::parse(this->dumpJSON("{\"mode\":\"flat\"}"));
        flatBlocks = flatObj["blocks"];
    }

    //output object
    json topObj;

    //create blocks map
    json &blocksObj = topObj["blocks"];
    for (const auto &block : getObjSetFromFlowList(flows))
    {
        //gather block info
        const std::string blockId = block.call("uid");
        auto &blockObj = blocksObj[blockId];

        //replace rendered names with names from flattened hierarchy
        blockObj["name"] = block.call<std::string>("getName");
        if (flatBlocks.count(blockId))
        {
            blockObj["name"] = flatBlocks[blockId]["name"];
        }

        //input port info
        json inputsArray;
        for (const auto &portInfo : block.call<std::vector<PortInfo>>("inputPortInfo"))
        {
            inputsArray.push_back(portInfoToObj(portInfo));
        }
        if (not inputsArray.empty()) blockObj["inputs"] = inputsArray;

        //output port info
        json outputsArray;
        for (const auto &portInfo : block.call<std::vector<PortInfo>>("outputPortInfo"))
        {
            outputsArray.push_back(portInfoToObj(portInfo));
        }
        if (not outputsArray.empty()) blockObj["outputs"] = outputsArray;

        //sub-topology info
        if (traverse and this->uid() != blockId) try
        {
            std::string subDump = block.call("dumpJSON", "{\"mode\":\"top\"}");
            auto subObj = json::parse(subDump);
            for (auto it = subObj.begin(); it != subObj.end(); ++it)
            {
                blockObj[it.key()] = subObj[it.key()];
            }
        }
        catch (const Pothos::Exception &){}
    }

    //create connections list
    json connsArray(json::array());
    for (const auto &flow : flows)
    {
        json connObj;
        connObj["srcId"] = flow.src.uid;
        connObj["srcName"] = flow.src.name;
        connObj["dstId"] = flow.dst.uid;
        connObj["dstName"] = flow.dst.name;
        connsArray.push_back(connObj);
    }
    topObj["connections"] = connsArray;

    //recursive flatten when instructed
    while (flatten and flattenDump(topObj));

    //return the string-formatted result
    return topObj.dump(4);
}
