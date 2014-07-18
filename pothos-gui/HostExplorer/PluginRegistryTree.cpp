// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "HostExplorer/PluginRegistryTree.hpp"
#include <Pothos/Remote.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Plugin.hpp>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include <Poco/Logger.h>
#include <iostream>

/***********************************************************************
 * recursive tree widget item convert dump into tree
 **********************************************************************/
class PluginPathTreeItem : public QTreeWidgetItem
{
public:
    template <typename Parent>
    PluginPathTreeItem(Parent *parent, const Pothos::PluginRegistryInfoDump &dump):
        QTreeWidgetItem(parent, columnsFromDump(dump))
    {
        for (const auto &subInfo : dump.subInfo)
        {
            new PluginPathTreeItem(this, subInfo);
        }
        this->setExpanded(dump.subInfo.size() < 21); //auto expand if not too many items
    }

private:
    static QStringList columnsFromDump(const Pothos::PluginRegistryInfoDump &dump)
    {
        QStringList columns;
        auto nodes = Pothos::PluginPath(dump.pluginPath).listNodes();

        if (nodes.empty()) columns.push_back("/");
        else columns.push_back(QString::fromStdString(nodes.back()));

        if (not dump.objectType.empty())
        {
            columns.push_back(QString::fromStdString(dump.objectType));
            columns.push_back(QString::fromStdString(dump.modulePath));
        }
        return columns;
    }
};

/***********************************************************************
 * information aquisition
 **********************************************************************/
static Pothos::PluginRegistryInfoDump getRegistryDump(const std::string &uriStr)
{
    try
    {
        auto env = Pothos::RemoteClient(uriStr).makeEnvironment("managed");
        return env->findProxy("Pothos/PluginRegistry").call<Pothos::PluginRegistryInfoDump>("dump");
    }
    catch (const Pothos::Exception &ex)
    {
        poco_error_f2(Poco::Logger::get("PothosGui.PluginRegistryTree"), "Failed to dump registry %s - %s", uriStr, ex.displayText());
    }
    return Pothos::PluginRegistryInfoDump();
}

/***********************************************************************
 * tree widget plugin registry implementation
 **********************************************************************/
PluginRegistryTree::PluginRegistryTree(QWidget *parent):
    QTreeWidget(parent),
    _watcher(new QFutureWatcher<Pothos::PluginRegistryInfoDump>(this))
{
    QStringList columnNames;
    columnNames.push_back(tr("Plugin path"));
    columnNames.push_back(tr("Object type"));
    columnNames.push_back(tr("Module path"));
    this->setColumnCount(columnNames.size());
    this->setHeaderLabels(columnNames);

    connect(
        _watcher, SIGNAL(finished(void)),
        this, SLOT(handleWatcherDone(void)));
}

void PluginRegistryTree::handeNodeInfoRequest(const std::string &uriStr)
{
    if (_watcher->isRunning()) return;
    while (this->topLevelItemCount() > 0) delete this->topLevelItem(0);
    _watcher->setFuture(QtConcurrent::run(std::bind(&getRegistryDump, uriStr)));
    emit startLoad();
}

void PluginRegistryTree::handleWatcherDone(void)
{
    new PluginPathTreeItem(this, _watcher->result());
    this->resizeColumnToContents(0);
    this->resizeColumnToContents(1);
    emit stopLoad();
}
