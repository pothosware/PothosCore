// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "BlockTree/BlockTreeWidgetItem.hpp"

void BlockTreeWidgetItem::load(const Poco::JSON::Object::Ptr &blockDesc, const std::string &category, const size_t depth)
{
    const auto slashIndex = category.find("/");
    const auto catName = category.substr(0, slashIndex);
    if (slashIndex == std::string::npos)
    {
        _blockDesc = blockDesc;
    }
    else
    {
        const auto catRest = category.substr(slashIndex+1);
        const auto key = catRest.substr(0, catRest.find("/"));
        if (_subNodes.find(key) == _subNodes.end())
        {
            _subNodes[key] = new BlockTreeWidgetItem(this, key);
            _subNodes[key]->setExpanded(depth < 2);
        }
        _subNodes[key]->load(blockDesc, catRest, depth+1);
    }
}

//this sets a tool tip -- but only when requested
QVariant BlockTreeWidgetItem::data(int column, int role) const
{
    if (role == Qt::ToolTipRole)
    {
        //sorry about this cast
        const_cast<BlockTreeWidgetItem *>(this)->setToolTipOnRequest();
    }
    return QTreeWidgetItem::data(column, role);
}

void BlockTreeWidgetItem::setToolTipOnRequest(void)
{
    const auto doc = extractDocString(_blockDesc);
    if (doc.isEmpty()) return;
    this->setToolTip(0, doc);
}

QString BlockTreeWidgetItem::extractDocString(Poco::JSON::Object::Ptr blockDesc)
{
    if (not blockDesc or not blockDesc->isArray("docs")) return "";
    QString output;
    output += "<b>" + QString::fromStdString(blockDesc->get("name").convert<std::string>()) + "</b>";
    output += "<p>";
    for (const auto &lineObj : *blockDesc->getArray("docs"))
    {
        const auto line = lineObj.extract<std::string>();
        if (line.empty()) output += "<p /><p>";
        else output += QString::fromStdString(line)+"\n";
    }
    output += "</p>";
    return "<div>" + output + "</div>";
}
