// Copyright (c) 2014-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/TopologyImpl.hpp"
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/Element.h>
#include <Poco/DOM/Text.h>
#include <Poco/DOM/DOMWriter.h>
#include <Poco/XML/XMLWriter.h>
#include <Poco/AutoPtr.h>
#include <Poco/NumberParser.h>
#include <sstream>

static Poco::AutoPtr<Poco::XML::Element> portInfoToElem(Poco::AutoPtr<Poco::XML::Document> xmlDoc, const Poco::JSON::Array::Ptr &portsInfo, const std::string &prefix)
{
    auto nodeTd = xmlDoc->createElement("td");
    nodeTd->setAttribute("border", "0");
    auto table = xmlDoc->createElement("table");
    nodeTd->appendChild(table);
    table->setAttribute("border", "0");
    table->setAttribute("cellspacing", "0");

    for (size_t i = 0; i < portsInfo->size(); i++)
    {
        const auto portInfo = portsInfo->getObject(i);
        auto name = portInfo->getValue<std::string>("name");
        auto isSigSlot = portInfo->getValue<bool>("isSigSlot");

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

static Poco::JSON::Array::Ptr getConnectedPortInfos(
    const Poco::JSON::Object::Ptr &topObj,
    const std::string &blockId,
    const bool enbFilter,
    const bool isInput)
{
    const auto connsArray = topObj->getArray("connections");
    const auto blocksObj = topObj->getObject("blocks");
    const auto blockObj = blocksObj->getObject(blockId);

    //grab the raw ports info
    Poco::JSON::Array::Ptr portsInfo(new Poco::JSON::Array());
    if (isInput and blockObj->has("inputs")) portsInfo = blockObj->getArray("inputs");
    if (not isInput and blockObj->has("outputs")) portsInfo = blockObj->getArray("outputs");

    //no filtering? return ASAP
    if (not enbFilter) return portsInfo;

    Poco::JSON::Array::Ptr filteredPortsInfo(new Poco::JSON::Array());
    for (size_t i = 0; i < portsInfo->size(); i++)
    {
        const auto portInfo = portsInfo->getObject(i);
        for (size_t c_i = 0; c_i < connsArray->size(); c_i++)
        {
            const auto conn = connsArray->getObject(c_i);
            if (
                (not isInput and blockId == conn->getValue<std::string>("srcId") and portInfo->getValue<std::string>("name") == conn->getValue<std::string>("srcName")) or
                (isInput and blockId == conn->getValue<std::string>("dstId") and portInfo->getValue<std::string>("name") == conn->getValue<std::string>("dstName"))
            )
            {
                filteredPortsInfo->add(portInfo);
                break;
            }
        }
    }
    return filteredPortsInfo;
}

std::string Pothos::Topology::toDotMarkup(const std::string &request)
{
    //parse request arguments
    const auto result = Poco::JSON::Parser().parse(request.empty()?"{}":request);
    auto configObj = result.extract<Poco::JSON::Object::Ptr>();
    const auto portConfig = configObj->optValue<std::string>("port", "connected");

    //get a JSON dump of the topology
    const auto dumpResult = Poco::JSON::Parser().parse(this->dumpJSON(request));
    const auto topObj = dumpResult.extract<Poco::JSON::Object::Ptr>();
    const auto connsArray = topObj->getArray("connections");
    const auto blocksObj = topObj->getObject("blocks");
    std::vector<std::string> blockIds; blocksObj->getNames(blockIds);

    std::ostringstream os;
    os << "digraph flat_flows {" << std::endl;
    os << "    rankdir=LR;" << std::endl;
    os << "    node [shape=record, fontsize=10];" << std::endl;

    for (const auto &blockId : blockIds)
    {
        const auto blockObj = blocksObj->getObject(blockId);

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

        if (inputPorts->size() > 0)
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
            auto name = blockObj->getValue<std::string>("name");
            if (name.empty()) name = "Empty Name";
            td->appendChild(xmlDoc->createTextNode(name));
        }
        if (outputPorts->size() > 0)
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

    for (size_t c_i = 0; c_i < connsArray->size(); c_i++)
    {
        const auto conn = connsArray->getObject(c_i);
        os << "    ";
        os << std::hash<std::string>()(conn->getValue<std::string>("srcId"));
        os << ":__out__" << conn->getValue<std::string>("srcName");
        os << " -> ";
        os << std::hash<std::string>()(conn->getValue<std::string>("dstId"));
        os << ":__in__" << conn->getValue<std::string>("dstName");
        os << ";" << std::endl;
    }

    os << "}" << std::endl;
    return os.str();
}
