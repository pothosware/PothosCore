// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/TopologyImpl.hpp>
#include "Framework/TopologyImpl.hpp"
#include <Pothos/Proxy.hpp>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>
#include <sstream>
#include <iostream>

static bool blockIsHier(const Poco::JSON::Object::Ptr &blockObj)
{
    return blockObj->has("connections");
}

static std::vector<std::pair<std::string, std::string>> resolvePorts(
    const Poco::JSON::Object::Ptr &topObj,
    const std::string &blockId,
    const std::string &portName,
    const bool resolveSrc
)
{
    std::vector<std::pair<std::string, std::string>> results;

    const auto blocksObj = topObj->getObject("blocks");
    const auto blockObj = blocksObj->getObject(blockId);

    if (not blockIsHier(blockObj))
    {
        results.push_back(std::make_pair(blockId, portName));
        return results;
    }

    const auto connsArray = blockObj->getArray("connections");
    for (size_t c_i = 0; c_i < connsArray->size(); c_i++)
    {
        const auto subConnObj = connsArray->getObject(c_i);
        std::string subId, subName;

        if (resolveSrc)
        {
            if (subConnObj->getValue<std::string>("dstId") != blockId) continue;
            if (subConnObj->getValue<std::string>("dstName") != portName) continue;
            subId = subConnObj->getValue<std::string>("srcId");
            subName = subConnObj->getValue<std::string>("srcName");
        }
        else
        {
            if (subConnObj->getValue<std::string>("srcId") != blockId) continue;
            if (subConnObj->getValue<std::string>("srcName") != portName) continue;
            subId = subConnObj->getValue<std::string>("dstId");
            subName = subConnObj->getValue<std::string>("dstName");
        }

        //ignore pass-through connections in this loop
        if (subId != blockId) results.push_back(std::make_pair(subId, subName));
    }

    return results;
}

static bool flattenDump(Poco::JSON::Object::Ptr &topObj)
{
    bool hierFound = false;

    //create new blocks object that flattens any hierarchy to 1 depth
    //if this block is a hierarchy -- bring its blocks to the top level
    const auto blocksObj = topObj->getObject("blocks");
    Poco::JSON::Object::Ptr flatBlocksObj(new Poco::JSON::Object());
    std::vector<std::string> blockUids; blocksObj->getNames(blockUids);
    for (const auto &uid : blockUids)
    {
        const auto blockObj = blocksObj->getObject(uid);
        if (blockIsHier(blockObj))
        {
            hierFound = true;
            const auto subBlocksObj = blockObj->getObject("blocks");
            const auto thisName = blockObj->getValue<std::string>("name");
            std::vector<std::string> subBlockUids; subBlocksObj->getNames(subBlockUids);
            for (const auto &subUid : subBlockUids)
            {
                auto subBlockObj = subBlocksObj->getObject(subUid);
                const auto subName = subBlockObj->getValue<std::string>("name");
                subBlockObj->set("name", thisName+"/"+subName); //heritage name
                flatBlocksObj->set(subUid, subBlockObj);
            }
        }
        else flatBlocksObj->set(uid, blockObj);
    }

    //create new connections array folding out depth 1 hierarchies
    const auto connsArray = topObj->getArray("connections");
    Poco::JSON::Array::Ptr flatConnsArray(new Poco::JSON::Array());
    for (size_t c_i = 0; c_i < connsArray->size(); c_i++)
    {
        const auto connObj = connsArray->getObject(c_i);
        for (const auto & resolvedSrc : resolvePorts(topObj, connObj->getValue<std::string>("srcId"), connObj->getValue<std::string>("srcName"), true))
        {
            for (const auto & resolvedDst : resolvePorts(topObj, connObj->getValue<std::string>("dstId"), connObj->getValue<std::string>("dstName"), false))
            {
                Poco::JSON::Object::Ptr flatConnObj(new Poco::JSON::Object());
                flatConnsArray->add(flatConnObj);
                flatConnObj->set("srcId", resolvedSrc.first);
                flatConnObj->set("srcName", resolvedSrc.second);
                flatConnObj->set("dstId", resolvedDst.first);
                flatConnObj->set("dstName", resolvedDst.second);
            }
        }
    }

    //resolve pass-through connections and totally internal connections
    for (const auto &uid : blockUids)
    {
        const auto blockObj = blocksObj->getObject(uid);
        if (not blockIsHier(blockObj)) continue;
        const auto subConnsArray = blockObj->getArray("connections");

        for (size_t c_i = 0; c_i < subConnsArray->size(); c_i++)
        {
            const auto subConnObj = subConnsArray->getObject(c_i);
            const bool srcIsThis = subConnObj->getValue<std::string>("srcId") == uid;
            const bool dstIsThis = subConnObj->getValue<std::string>("dstId") == uid;

            //totally internal connection
            if (not srcIsThis and not dstIsThis) flatConnsArray->add(subConnObj);

            //otherwise not a pass-through
            if (not srcIsThis or not dstIsThis) continue;

            //find sources where the destination is this pass-through
            for (size_t c_s = 0; c_s < connsArray->size(); c_s++)
            {
                const auto connObj_s = connsArray->getObject(c_s);
                if (connObj_s->getValue<std::string>("dstId") != uid) continue;
                if (connObj_s->getValue<std::string>("dstName") != subConnObj->getValue<std::string>("srcName")) continue;

                //find sources where the destination is this pass-through
                for (size_t c_d = 0; c_d < connsArray->size(); c_d++)
                {
                    const auto connObj_d = connsArray->getObject(c_d);
                    if (connObj_d->getValue<std::string>("srcId") != uid) continue;
                    if (connObj_d->getValue<std::string>("srcName") != subConnObj->getValue<std::string>("dstName")) continue;

                    Poco::JSON::Object::Ptr flatConnObj(new Poco::JSON::Object());
                    flatConnsArray->add(flatConnObj);
                    flatConnObj->set("srcId", connObj_s->get("srcId"));
                    flatConnObj->set("srcName", connObj_s->get("srcName"));
                    flatConnObj->set("dstId", connObj_d->get("dstId"));
                    flatConnObj->set("dstName", connObj_d->get("dstName"));
                }
            }
        }
    }

    //set new flat data into the top object
    topObj->set("blocks", flatBlocksObj);
    topObj->set("connections", flatConnsArray);
    return hierFound;
}

static Poco::JSON::Object::Ptr portInfoToObj(const Pothos::PortInfo &portInfo)
{
    Poco::JSON::Object::Ptr infoObj(new Poco::JSON::Object());
    infoObj->set("name", portInfo.name);
    infoObj->set("dtype", portInfo.dtype.toMarkup());
    infoObj->set("size", Poco::UInt64(portInfo.dtype.size()));
    infoObj->set("isSigSlot", portInfo.isSigSlot);
    return infoObj;
}

std::string Pothos::Topology::dumpJSON(const std::string &request)
{
    //extract input request
    Poco::JSON::Parser p; p.parse(request.empty()?"{}":request);
    auto configObj = p.getHandler()->asVar().extract<Poco::JSON::Object::Ptr>();
    const auto modeConfig = configObj->optValue<std::string>("mode", "flat");

    //parse request into traversal arguments
    const bool flatten = (modeConfig == "flat");
    const bool traverse = (modeConfig != "rendered");
    const auto &flows = (modeConfig == "rendered")?_impl->activeFlatFlows:_impl->flows;

    //replace rendered names with names from flattened hierarchy
    Poco::JSON::Object::Ptr flatBlocks;
    if (modeConfig == "rendered")
    {
        Poco::JSON::Parser pFlat; pFlat.parse(this->dumpJSON("{\"mode\":\"flat\"}"));
        auto flatObj = pFlat.getHandler()->asVar().extract<Poco::JSON::Object::Ptr>();
        flatBlocks = flatObj->getObject("blocks");
    }

    //output object
    Poco::JSON::Object::Ptr topObj(new Poco::JSON::Object());

    //create blocks map
    Poco::JSON::Object::Ptr blocksObj(new Poco::JSON::Object());
    topObj->set("blocks", blocksObj);
    for (const auto &block : getObjSetFromFlowList(flows))
    {
        //gather block info
        Poco::JSON::Object::Ptr blockObj(new Poco::JSON::Object());
        const auto blockId = block.call<std::string>("uid");
        blocksObj->set(blockId, blockObj);

        //replace rendered names with names from flattened hierarchy
        blockObj->set("name", block.call<std::string>("getName"));
        if (flatBlocks and flatBlocks->has(blockId))
        {
            blockObj->set("name", flatBlocks->getObject(blockId)->getValue<std::string>("name"));
        }

        //input port info
        Poco::JSON::Array::Ptr inputsArray(new Poco::JSON::Array());
        for (const auto &portInfo : block.call<std::vector<PortInfo>>("inputPortInfo"))
        {
            inputsArray->add(portInfoToObj(portInfo));
        }
        if (inputsArray->size() > 0) blockObj->set("inputs", inputsArray);

        //output port info
        Poco::JSON::Array::Ptr outputsArray(new Poco::JSON::Array());
        for (const auto &portInfo : block.call<std::vector<PortInfo>>("outputPortInfo"))
        {
            outputsArray->add(portInfoToObj(portInfo));
        }
        if (outputsArray->size() > 0) blockObj->set("outputs", outputsArray);

        //sub-topology info
        if (traverse and this->uid() != blockId) try
        {
            auto subDump = block.call<std::string>("dumpJSON", "{\"mode\":\"top\"}");
            Poco::JSON::Parser psub; psub.parse(subDump);
            auto subObj = psub.getHandler()->asVar().extract<Poco::JSON::Object::Ptr>();
            std::vector<std::string> names; subObj->getNames(names);
            for (const auto &name : names) blockObj->set(name, subObj->get(name));
        }
        catch (const Pothos::Exception &){}
    }

    //create connections list
    Poco::JSON::Array::Ptr connsArray(new Poco::JSON::Array());
    topObj->set("connections", connsArray);
    for (const auto &flow : flows)
    {
        Poco::JSON::Object::Ptr connObj(new Poco::JSON::Object());
        connsArray->add(connObj);
        connObj->set("srcId", flow.src.uid);
        connObj->set("srcName", flow.src.name);
        connObj->set("dstId", flow.dst.uid);
        connObj->set("dstName", flow.dst.name);
    }

    //recursive flatten when instructed
    while (flatten and flattenDump(topObj));

    //return the string-formatted result
    std::stringstream ss; topObj->stringify(ss, 4);
    return ss.str();
}
