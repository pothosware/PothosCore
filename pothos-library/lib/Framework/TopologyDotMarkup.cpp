// Copyright (c) 2014-2014 Josh Blum
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

std::string Pothos::Topology::toDotMarkup(const bool flat)
{
    std::ostringstream os;
    auto flows = (flat)? _impl->activeFlatFlows : _impl->flows;
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

        const auto inputInfo = block.call<std::vector<Pothos::PortInfo>>("inputPortInfo");
        const auto outputInfo = block.call<std::vector<Pothos::PortInfo>>("outputPortInfo");

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
            td->appendChild(xmlDoc->createTextNode(block.call<std::string>("getName")));
        }
        if (not outputInfo.empty()) nodeTr->appendChild(portInfoToElem(xmlDoc, outputInfo, "out"));

        //dot node entry
        os << "    ";
        os << block.callProxy("uid").hashCode();
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
        os << flow.src.obj.callProxy("uid").hashCode() << ":__out__" << flow.src.name;
        os << " -> ";
        os << flow.dst.obj.callProxy("uid").hashCode() << ":__in__" << flow.dst.name;
        os << ";" << std::endl;
    }

    os << "}" << std::endl;
    return os.str();
}
