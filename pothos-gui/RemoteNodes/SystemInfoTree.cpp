// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGui.hpp"
#include <QTreeWidget>
#include <Pothos/Remote.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/System/Info.hpp>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include <Poco/Logger.h>

/***********************************************************************
 * information aquisition
 **********************************************************************/
struct InfoResult
{
    Pothos::System::NodeInfo nodeInfo;
    std::vector<Pothos::System::CpuInfo> cpuInfo;
};

static InfoResult getInfo(const Pothos::RemoteNode &node)
{
    InfoResult info;
    try
    {
        auto env = Pothos::RemoteNode(node).makeClient("info").makeEnvironment("managed");
        info.nodeInfo = env->findProxy("Pothos/System/NodeInfo").call<Pothos::System::NodeInfo>("get");
        info.cpuInfo = env->findProxy("Pothos/System/CpuInfo").call<std::vector<Pothos::System::CpuInfo>>("get");
    }
    catch (const Pothos::Exception &ex)
    {
        poco_error_f2(Poco::Logger::get("PothosGui.SystemInfoTree"), "Failed to query system info %s - %s", node.getUri(), ex.displayText());
    }
    return info;
}

/***********************************************************************
 * system info tree widget
 **********************************************************************/
class SystemInfoTree : public QTreeWidget
{
    Q_OBJECT
public:
    SystemInfoTree(QWidget *parent):
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

signals:
    void startLoad(void);
    void stopLoad(void);

private slots:

    void handeNodeInfoRequest(const Pothos::RemoteNode &node)
    {
        if (_watcher->isRunning()) return;
        while (this->topLevelItemCount() > 0) delete this->topLevelItem(0);
        _watcher->setFuture(QtConcurrent::run(std::bind(&getInfo, node)));
        emit startLoad();
    }

    void handleWatcherDone(void)
    {
        const auto info = _watcher->result();

        const auto &nodeInfo = info.nodeInfo;
        {
            QStringList columns;
            columns.push_back(tr("Node Info"));
            auto rootItem = new QTreeWidgetItem(this, columns);
            rootItem->setExpanded(true);
            makeEntry(rootItem, "OS Name", nodeInfo.osName);
            makeEntry(rootItem, "OS Version", nodeInfo.osVersion);
            makeEntry(rootItem, "OS Architecture", nodeInfo.osArchitecture);
            makeEntry(rootItem, "Node Name", nodeInfo.nodeName);
            makeEntry(rootItem, "Node ID", nodeInfo.nodeId);
        }

        const auto &cpuInfo = info.cpuInfo;
        for (size_t i = 0; i < cpuInfo.size(); i++)
        {
            QStringList columns;
            columns.push_back(tr("CPU %1 Info").arg(i));
            auto rootItem = new QTreeWidgetItem(this, columns);
            rootItem->setExpanded(i == 0);
            //makeEntry(rootItem, "Current Speed", std::to_string(cpuInfo[i].mhz), "MHz");
            makeEntry(rootItem, "Min Speed", std::to_string(cpuInfo[i].mhzMin), "MHz");
            makeEntry(rootItem, "Max Speed", std::to_string(cpuInfo[i].mhzMax), "MHz");
            makeEntry(rootItem, "Cache Size", std::to_string(cpuInfo[i].cacheSize), "KB");
            makeEntry(rootItem, "Num Sockets", std::to_string(cpuInfo[i].totalSockets));
            makeEntry(rootItem, "Num Cores", std::to_string(cpuInfo[i].totalCores));
            //makeEntry(rootItem, "Cores Per Socket", std::to_string(cpuInfo[i].coresPerSocket));
        }

        this->resizeColumnToContents(0);
        this->resizeColumnToContents(1);
        this->resizeColumnToContents(2);
        emit stopLoad();
    }

private:

    template <typename Parent>
    static QTreeWidgetItem *makeEntry(Parent *root, const char *name, const std::string &value, const char *unit = "")
    {
        QStringList columns;
        columns.push_back(tr(name));
        columns.push_back(QString::fromStdString(value));
        columns.push_back(unit);
        return new QTreeWidgetItem(root, columns);
    }

    QFutureWatcher<InfoResult> *_watcher;
};

QWidget *makeSystemInfoTree(QWidget *parent)
{
    return new SystemInfoTree(parent);
}

#include "SystemInfoTree.moc"
