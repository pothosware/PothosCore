// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "HostExplorer/SystemInfoTree.hpp"
#include <Pothos/Remote.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/System.hpp>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include <Poco/Logger.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>

/***********************************************************************
 * information aquisition
 **********************************************************************/
static InfoResult getInfo(const std::string &uriStr)
{
    InfoResult info;
    try
    {
        auto env = Pothos::RemoteClient(uriStr).makeEnvironment("managed");
        info.hostInfo = env->findProxy("Pothos/System/HostInfo").call<Pothos::System::HostInfo>("get");
        info.numaInfo = env->findProxy("Pothos/System/NumaInfo").call<std::vector<Pothos::System::NumaInfo>>("get");
        auto deviceInfo = env->findProxy("Pothos/Util/DeviceInfoUtils").call<std::string>("dumpJson");
        Poco::JSON::Parser p; p.parse(deviceInfo);
        info.deviceInfo = p.getHandler()->asVar().extract<Poco::JSON::Array::Ptr>();
    }
    catch (const Pothos::Exception &ex)
    {
        poco_error_f2(Poco::Logger::get("PothosGui.SystemInfoTree"), "Failed to query system info %s - %s", uriStr, ex.displayText());
    }
    return info;
}

/***********************************************************************
 * system info tree widget implementation
 **********************************************************************/
SystemInfoTree::SystemInfoTree(QWidget *parent):
    QTreeWidget(parent),
    _watcher(new QFutureWatcher<InfoResult>(this))
{
    QStringList columnNames;
    columnNames.push_back(tr("Name"));
    columnNames.push_back(tr("Value"));
    columnNames.push_back(tr("Unit"));
    this->setColumnCount(columnNames.size());
    this->setHeaderLabels(columnNames);

    connect(
        _watcher, SIGNAL(finished(void)),
        this, SLOT(handleWatcherDone(void)));
}

void SystemInfoTree::handeInfoRequest(const std::string &uriStr)
{
    if (_watcher->isRunning()) return;
    while (this->topLevelItemCount() > 0) delete this->topLevelItem(0);
    _watcher->setFuture(QtConcurrent::run(std::bind(&getInfo, uriStr)));
    emit startLoad();
}

void SystemInfoTree::handleWatcherDone(void)
{
    const auto info = _watcher->result();

    const auto &hostInfo = info.hostInfo;
    {
        QStringList columns;
        columns.push_back(tr("Host Info"));
        auto rootItem = new QTreeWidgetItem(this, columns);
        rootItem->setExpanded(true);
        makeEntry(rootItem, "OS Name", hostInfo.osName);
        makeEntry(rootItem, "OS Version", hostInfo.osVersion);
        makeEntry(rootItem, "OS Architecture", hostInfo.osArchitecture);
        makeEntry(rootItem, "Node Name", hostInfo.nodeName);
        makeEntry(rootItem, "Node ID", hostInfo.nodeId);
        makeEntry(rootItem, "Processors", std::to_string(hostInfo.processorCount), "CPUs");
    }

    for (const auto &numaInfo : info.numaInfo)
    {
        QStringList columns;
        columns.push_back(tr("NUMA Node %1 Info").arg(numaInfo.nodeNumber));
        auto rootItem = new QTreeWidgetItem(this, columns);
        rootItem->setExpanded(numaInfo.nodeNumber == 0);
        if (numaInfo.totalMemory != 0) makeEntry(rootItem, "Total Memory", std::to_string(numaInfo.totalMemory/1024/1024), "MB");
        if (numaInfo.freeMemory != 0) makeEntry(rootItem, "Free Memory", std::to_string(numaInfo.freeMemory/1024/1024), "MB");
        std::string cpuStr;
        for (auto i : numaInfo.cpus)
        {
            if (not cpuStr.empty()) cpuStr += ", ";
            cpuStr += std::to_string(i);
        }
        makeEntry(rootItem, "CPUs", cpuStr);
    }

    //adjust value column before arbitrary values from device info
    this->resizeColumnToContents(1);

    if (info.deviceInfo) for (size_t i = 0; i < info.deviceInfo->size(); i++)
    {
        this->loadJsonObject(this, "", info.deviceInfo->getObject(i), true/*expand*/);
    }

    //adjust names and units columns after all information is loaded
    this->resizeColumnToContents(0);
    //this->resizeColumnToContents(1);
    this->resizeColumnToContents(2);
    emit stopLoad();
}
