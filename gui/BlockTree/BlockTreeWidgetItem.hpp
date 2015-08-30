// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QTreeWidgetItem>
#include <Poco/JSON/Object.h>
#include <string>
#include <map>

class BlockTreeWidgetItem : public QTreeWidgetItem
{
public:
    template <typename ParentType>
    BlockTreeWidgetItem(ParentType *parent, const std::string &name):
        QTreeWidgetItem(parent, QStringList(QString::fromStdString(name)))
    {
        return;
    }

    void load(const Poco::JSON::Object::Ptr &blockDesc, const std::string &category, const size_t depth = 0);

    Poco::JSON::Object::Ptr getBlockDesc(void) const
    {
        return _blockDesc;
    }

private:
    //this sets a tool tip -- but only when requested
    QVariant data(int column, int role) const;

    void setToolTipOnRequest(void);

    static QString extractDocString(Poco::JSON::Object::Ptr blockDesc);

    std::map<std::string, BlockTreeWidgetItem *> _subNodes;
    Poco::JSON::Object::Ptr _blockDesc;
};
