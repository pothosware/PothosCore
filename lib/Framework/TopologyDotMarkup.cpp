// Copyright (c) 2014-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/TopologyImpl.hpp"
#include <Poco/DOM/Document.h>
#include <Poco/DOM/Element.h>
#include <Poco/DOM/Text.h>
#include <Poco/DOM/DOMWriter.h>
#include <Poco/XML/XMLWriter.h>
#include <Poco/AutoPtr.h>
#include <Poco/NumberParser.h>
#include <sstream>
#include <json.hpp>

using json = nlohmann::json;

static Poco::AutoPtr<Poco::XML::Element> portInfoToElem(Poco::AutoPtr<Poco::XML::Document> xmlDoc, const json &portsInfo, const std::string &prefix)
{
    auto nodeTd = xmlDoc->createElement("td");
    nodeTd->setAttribute("border", "0");
    auto table = xmlDoc->createElement("table");
    nodeTd->appendChild(table);
    table->setAttribute("border", "0");
    table->setAttribute("cellspacing", "0");

    for (const auto &portInfo : portsInfo)
    {
        std::string name = portInfo["name"];
        const bool isSigSlot = portInfo["isSigSlot"];

        auto tr = xmlDoc->createElement("tr");
        table->appendChild(tr);
        auto td = xmlDoc->createElement("td");
        tr->appendChild(td);
        td->setAttribute("border", "1");
        if (prefix == "in"  and isSigSlot)     td->setAttribute("bgcolor", "#AEC6CF");
        if (prefix == "in"  and not isSigSlot) td->setAttribute("bgcolor", "#779ECB");
        if (prefix == "out" and isSigSlot)     td->setAttribute("bgcolor", "#77DD77");
        if (prefix == "out" and not isSigSlot) td->setAttribute("bgcolor", "#03C03C");
        td->setAttribute("port", "__"+prefix+"__"+name);
        unsigned value = 0;
        if (Poco::NumberParser::tryParseUnsigned(name, value)) name = prefix+name;
        td->appendChild(xmlDoc->createTextNode(name));
    }

    return nodeTd;
}

static json getConnectedPortInfos(
    const json &topObj,
    const std::string &blockId,
    const bool enbFilter,
    const bool isInput)
{
    const auto connsArray = topObj["connections"];
    const auto blocksObj = topObj["blocks"];
    const auto blockObj = blocksObj[blockId];

    //grab the raw ports info
    auto portsInfo = json::array();
    if (isInput and blockObj.count("inputs")) portsInfo = blockObj["inputs"];
    if (not isInput and blockObj.count("outputs")) portsInfo = blockObj["outputs"];

    //no filtering? return ASAP
    if (not enbFilter) return portsInfo;

    json filteredPortsInfo;
    for (const auto &portInfo : portsInfo)
    {
        for (const auto &conn : connsArray)
        {
            if (
                (not isInput and blockId == conn["srcId"].get<std::string>() and portInfo["name"].get<std::string>() == conn["srcName"].get<std::string>()) or
                (isInput and blockId == conn["dstId"].get<std::string>() and portInfo["name"].get<std::string>() == conn["dstName"].get<std::string>())
            )
            {
                filteredPortsInfo.push_back(portInfo);
                break;
            }
        }
    }
    return filteredPortsInfo;
}

std::string Pothos::Topology::toDotMarkup(const std::string &request)
{
    //parse request arguments
    const auto configObj = json::parse(request.empty()?"{}":request);
    const auto portConfig = configObj.value<std::string>("port", "connected");

    //get a JSON dump of the topology
    const auto topObj = json::parse(this->dumpJSON(request));
    const auto connsArray = topObj["connections"];
    const auto blocksObj = topObj["blocks"];

    std::ostringstream os;
    os << "digraph flat_flows {" << std::endl;
    os << "    rankdir=LR;" << std::endl;
    os << "    node [shape=record, fontsize=10];" << std::endl;

    for (auto it = blocksObj.begin(); it != blocksObj.end(); ++it)
    {
        const auto blockId = it.key();
        const auto blockObj = it.value();

        //form xml
        Poco::AutoPtr<Poco::XML::Document> xmlDoc(new Poco::XML::Document());
        auto nodeTable = xmlDoc->createElement("table");
        xmlDoc->appendChild(nodeTable);
        nodeTable->setAttribute("border", "0");
        nodeTable->setAttribute("cellpadding", "0");
        nodeTable->setAttribute("cellspacing", "0");
        auto nodeTr = xmlDoc->createElement("tr");
        nodeTable->appendChild(nodeTr);

        const bool enbFilter = portConfig == "connected";
        const auto inputPorts = getConnectedPortInfos(topObj, blockId, enbFilter, true);
        const auto outputPorts = getConnectedPortInfos(topObj, blockId, enbFilter, false);

        if (not inputPorts.empty())
        {
            nodeTr->appendChild(portInfoToElem(xmlDoc, inputPorts, "in"));
        }
        {
            auto nodeTd = xmlDoc->createElement("td");
            nodeTd->setAttribute("border", "0");
            nodeTr->appendChild(nodeTd);
            auto table = xmlDoc->createElement("table");
            nodeTd->appendChild(table);
            table->setAttribute("border", "0");
            table->setAttribute("cellspacing", "0");
            auto tr = xmlDoc->createElement("tr");
            table->appendChild(tr);
            auto td = xmlDoc->createElement("td");
            td->setAttribute("border", "1");
            td->setAttribute("bgcolor", "azure");
            tr->appendChild(td);
            std::string name = blockObj["name"];
            if (name.empty()) name = "Empty Name";
            td->appendChild(xmlDoc->createTextNode(name));
        }
        if (not outputPorts.empty())
        {
            nodeTr->appendChild(portInfoToElem(xmlDoc, outputPorts, "out"));
        }

        //dot node entry
        os << "    ";
        os << std::hash<std::string>()(blockId);
        os << "[" << std::endl;
        os << "    shape=none," << std::endl;
        os << "    label=<" << std::endl;
        Poco::XML::DOMWriter write;
        write.setOptions(Poco::XML::XMLWriter::PRETTY_PRINT);
        write.writeNode(os, xmlDoc);
        os << "    >" << std::endl;
        os << "];" << std::endl;
    }

    for (const auto &conn : connsArray)
    {
        os << "    ";
        os << std::hash<std::string>()(conn["srcId"].get<std::string>());
        os << ":__out__" << conn["srcName"].get<std::string>();
        os << " -> ";
        os << std::hash<std::string>()(conn["dstId"].get<std::string>());
        os << ":__in__" << conn["dstName"].get<std::string>();
        os << ";" << std::endl;
    }

    os << "}" << std::endl;
    return os.str();
}
