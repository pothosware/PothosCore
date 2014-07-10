// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGui.hpp"
#include <QTreeWidget>
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
struct InfoResult
{
    Pothos::System::NodeInfo nodeInfo;
    std::vector<Pothos::System::NumaInfo> numaInfo;
    Poco::JSON::Array::Ptr deviceInfo;
};

static InfoResult getInfo(const Pothos::RemoteNode &node)
{
    InfoResult info;
    try
    {
        auto env = Pothos::RemoteNode(node).makeClient("info").makeEnvironment("managed");
        info.nodeInfo = env->findProxy("Pothos/System/NodeInfo").call<Pothos::System::NodeInfo>("get");
        info.numaInfo = env->findProxy("Pothos/System/NumaInfo").call<std::vector<Pothos::System::NumaInfo>>("get");
        auto deviceInfo = env->findProxy("Pothos/Util/DeviceInfoUtils").call<std::string>("dumpJson");
        Poco::JSON::Parser p; p.parse(deviceInfo);
        info.deviceInfo = p.getHandler()->asVar().extract<Poco::JSON::Array::Ptr>();
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
            makeEntry(rootItem, "Processors", std::to_string(nodeInfo.processorCount), "CPUs");
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

        for (size_t i = 0; i < info.deviceInfo->size(); i++)
        {
            this->loadJsonObject(this, "", info.deviceInfo->getObject(i), true/*expand*/);
        }

        //adjust names and units columns after all information is loaded
        this->resizeColumnToContents(0);
        //this->resizeColumnToContents(1);
        this->resizeColumnToContents(2);
        emit stopLoad();
    }

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

QWidget *makeSystemInfoTree(QWidget *parent)
{
    return new SystemInfoTree(parent);
}

#include "SystemInfoTree.moc"
