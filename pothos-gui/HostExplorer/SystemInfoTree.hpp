// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <QTreeWidget>
#include <QFutureWatcher>
#include <QTreeWidgetItem>
#include <QStringList>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Pothos/System.hpp>
#include <string>

struct InfoResult
{
    Pothos::System::NodeInfo nodeInfo;
    std::vector<Pothos::System::NumaInfo> numaInfo;
    Poco::JSON::Array::Ptr deviceInfo;
};

//! tree widget display for a host's system info
class SystemInfoTree : public QTreeWidget
{
    Q_OBJECT
public:
    SystemInfoTree(QWidget *parent);

signals:
    void startLoad(void);
    void stopLoad(void);

private slots:

    void handeNodeInfoRequest(const std::string &uriStr);

    void handleWatcherDone(void);

private:

    template <typename Parent>
    static QTreeWidgetItem *makeEntry(Parent *root, const std::string &name, const std::string &value, const char *unit = "")
    {
        QStringList columns;
        columns.push_back(QString::fromStdString(name));
        columns.push_back(QString::fromStdString(value));
        columns.push_back(unit);
        return new QTreeWidgetItem(root, columns);
    }

    template <typename Parent>
    void loadJsonObject(Parent *root, const std::string &rootName, const Poco::JSON::Object::Ptr &obj, const bool expand = false)
    {
        std::vector<std::string> names; obj->getNames(names);
        for (const auto &name : names)
        {
            std::string newName = name;
            if (not rootName.empty()) newName = rootName + " " + name;
            loadJsonVar(root, newName, obj->get(name), expand);
        }
    }

    template <typename Parent>
    void loadJsonArray(Parent *root, const std::string &rootName, const Poco::JSON::Array::Ptr &arr, const bool expand = false)
    {
        for (size_t i = 0; i < arr->size(); i++)
        {
            loadJsonVar(root, rootName + " " + std::to_string(i), arr->get(i), expand and (i == 0));
        }
    }

    template <typename Parent>
    void loadJsonVar(Parent *root, const std::string &rootName, const Poco::Dynamic::Var &var, const bool expand = false)
    {
        if (var.type() == typeid(Poco::JSON::Array::Ptr))
        {
            this->loadJsonArray(root, rootName, var.extract<Poco::JSON::Array::Ptr>(), expand);
        }
        else if (var.type() == typeid(Poco::JSON::Object::Ptr))
        {
            QStringList columns;
            columns.push_back(QString::fromStdString(rootName));
            auto rootItem = new QTreeWidgetItem(root, columns);
            rootItem->setExpanded(expand);
            this->loadJsonObject(rootItem, "", var.extract<Poco::JSON::Object::Ptr>());
        }
        else
        {
            auto entry = makeEntry(root, rootName, var.convert<std::string>());
            entry->setExpanded(expand);
        }
    }

    QFutureWatcher<InfoResult> *_watcher;
};
