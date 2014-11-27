// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/TopologyImpl.hpp"
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

static Poco::AutoPtr<Poco::XML::Element> portInfoToElem(Poco::AutoPtr<Poco::XML::Document> xmlDoc, const std::vector<Pothos::PortInfo> &portInfo, const std::string &prefix)
{
    auto nodeTd = xmlDoc->createElement("td");
    nodeTd->setAttribute("border", "0");
    auto table = xmlDoc->createElement("table");
    nodeTd->appendChild(table);
    table->setAttribute("border", "0");
    table->setAttribute("cellspacing", "0");

    for (const auto &info : portInfo)
    {
        auto tr = xmlDoc->createElement("tr");
        table->appendChild(tr);
        auto td = xmlDoc->createElement("td");
        tr->appendChild(td);
        td->setAttribute("border", "1");
        if (prefix == "in"  and info.isSigSlot)     td->setAttribute("bgcolor", "#AEC6CF");
        if (prefix == "in"  and not info.isSigSlot) td->setAttribute("bgcolor", "#779ECB");
        if (prefix == "out" and info.isSigSlot)     td->setAttribute("bgcolor", "#77DD77");
        if (prefix == "out" and not info.isSigSlot) td->setAttribute("bgcolor", "#03C03C");
        td->setAttribute("port", "__"+prefix+"__"+info.name);
        unsigned value = 0;
        auto name = info.name;
        if (Poco::NumberParser::tryParseUnsigned(info.name, value)) name = prefix+name;
        td->appendChild(xmlDoc->createTextNode(name));
    }

    return nodeTd;
}

static std::vector<Pothos::PortInfo> getConnectedPortInfos(
    const Pothos::Proxy &block,
    const std::vector<Flow> &flows,
    const std::string &getter,
    const bool enbFilter,
    const bool isInput)
{
    const auto uid = block.call<std::string>("uid");
    const auto inputInfo = block.call<std::vector<Pothos::PortInfo>>(getter);

    if (not enbFilter) return inputInfo;

    std::vector<Pothos::PortInfo> outputInfo;
    for (const auto &info : inputInfo)
    {
        for (const auto &flow : flows)
        {
            if (
                (not isInput and uid == flow.src.uid and info.name == flow.src.name) or
                (isInput and uid == flow.dst.uid and info.name == flow.dst.name)
            )
            {
                outputInfo.push_back(info);
                break;
            }
        }
    }
    return outputInfo;
}

std::string Pothos::Topology::toDotMarkup(const std::string &config)
{
    Poco::JSON::Parser p; p.parse(config.empty()?"{}":config);
    auto configObj = p.getHandler()->asVar().extract<Poco::JSON::Object::Ptr>();
    const auto modeConfig = configObj->optValue<std::string>("mode", "flat");
    const auto portConfig = configObj->optValue<std::string>("port", "connected");

    std::ostringstream os;
    auto flows = (modeConfig == "flat")? _impl->activeFlatFlows : _impl->flows;
    auto blocks = getObjSetFromFlowList(flows);

    os << "digraph flat_flows {" << std::endl;
    os << "    rankdir=LR;" << std::endl;
    os << "    node [shape=record, fontsize=10];" << std::endl;

    for (const auto &block : blocks)
    {
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
        const auto inputInfo = getConnectedPortInfos(block, flows, "inputPortInfo", enbFilter, true);
        const auto outputInfo = getConnectedPortInfos(block, flows, "outputPortInfo", enbFilter, false);

        if (not inputInfo.empty()) nodeTr->appendChild(portInfoToElem(xmlDoc, inputInfo, "in"));
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
            auto name = block.call<std::string>("getName");
            if (name.empty()) name = "Empty Name";
            td->appendChild(xmlDoc->createTextNode(name));
        }
        if (not outputInfo.empty()) nodeTr->appendChild(portInfoToElem(xmlDoc, outputInfo, "out"));

        //dot node entry
        os << "    ";
        os << std::hash<std::string>()(block.call<std::string>("uid"));
        os << "[" << std::endl;
        os << "    shape=none," << std::endl;
        os << "    label=<" << std::endl;
        Poco::XML::DOMWriter write;
        write.setOptions(Poco::XML::XMLWriter::PRETTY_PRINT);
        write.writeNode(os, xmlDoc);
        os << "    >" << std::endl;
        os << "];" << std::endl;
    }

    for (const auto &flow : flows)
    {
        os << "    ";
        os << std::hash<std::string>()(flow.src.uid) << ":__out__" << flow.src.name;
        os << " -> ";
        os << std::hash<std::string>()(flow.dst.uid) << ":__in__" << flow.dst.name;
        os << ";" << std::endl;
    }

    os << "}" << std::endl;
    return os.str();
}
