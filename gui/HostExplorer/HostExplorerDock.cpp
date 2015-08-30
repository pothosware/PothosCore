// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGuiUtils.hpp" //icon path
#include "HostExplorer/HostExplorerDock.hpp"
#include "HostExplorer/HostSelectionTable.hpp"
#include "HostExplorer/PluginModuleTree.hpp"
#include "HostExplorer/PluginRegistryTree.hpp"
#include "HostExplorer/SystemInfoTree.hpp"
#include <QVBoxLayout>
#include <QTabWidget>
#include <QLabel>
#include <QMovie>
#include <QTabBar>
#include <QSignalMapper>
#include <Pothos/Remote.hpp>
#include <iostream>

/***********************************************************************
 * top level window for host info implementation
 **********************************************************************/
HostExplorerDock::HostExplorerDock(QWidget *parent):
    QDockWidget(parent),
    _table(new HostSelectionTable(this)),
    _tabs(new QTabWidget(this)),
    _startMapper(new QSignalMapper(this)),
    _stopMapper(new QSignalMapper(this))
{
    this->setObjectName("HostExplorerDock");
    this->setWindowTitle(tr("Host Explorer"));
    this->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    this->setWidget(new QWidget(this));

    auto layout = new QVBoxLayout(this->widget());
    layout->addWidget(_table);

    _tabs->addTab(new SystemInfoTree(_tabs), tr("System Info"));
    _tabs->addTab(new PluginRegistryTree(_tabs), tr("Plugin Registry"));
    _tabs->addTab(new PluginModuleTree(_tabs), tr("Plugin Modules"));
    layout->addWidget(_tabs, 1);

    //connect mappers
    connect(_startMapper, SIGNAL(mapped(const int)), this, SLOT(start(const int)));
    connect(_stopMapper, SIGNAL(mapped(const int)), this, SLOT(stop(const int)));
    connect(_table, SIGNAL(hostUriListChanged(void)), this, SIGNAL(hostUriListChanged(void)));

    //connect handlers for node info so the tabs update
    for (int i = 0; i < _tabs->count(); i++)
    {
        connect(
            _table, SIGNAL(hostInfoRequest(const std::string &)),
            _tabs->widget(i), SLOT(handeInfoRequest(const std::string &)));

        connect(_tabs->widget(i), SIGNAL(startLoad(void)), _startMapper, SLOT(map(void)));
        _startMapper->setMapping(_tabs->widget(i), i);

        connect(_tabs->widget(i), SIGNAL(stopLoad(void)), _stopMapper, SLOT(map(void)));
        _stopMapper->setMapping(_tabs->widget(i), i);
    }
}

QStringList HostExplorerDock::hostUriList(void) const
{
    return _table->hostUriList();
}

void HostExplorerDock::start(const int index)
{
    auto label = new QLabel(_tabs);
    auto movie = new QMovie(makeIconPath("loading.gif"), QByteArray(), label);
    label->setMovie(movie);
    movie->start();
    _tabs->tabBar()->setTabButton(index, QTabBar::LeftSide, label);
}

void HostExplorerDock::stop(const int index)
{
    _tabs->tabBar()->setTabButton(index, QTabBar::LeftSide, new QLabel(_tabs));
}
