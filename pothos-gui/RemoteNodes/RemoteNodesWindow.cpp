// Copyright (c) 2013-2013 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGui.hpp"
#include <QVBoxLayout>
#include <QTabWidget>
#include <QLabel>
#include <QMovie>
#include <QTabBar>
#include <QSignalMapper>
#include <Pothos/Remote.hpp>
#include <iostream>

/***********************************************************************
 * tab widget overload to access tabbar
 **********************************************************************/
class RemoteNodeInfoTabs : public QTabWidget
{
    Q_OBJECT
public:
    RemoteNodeInfoTabs(QWidget *parent):
        QTabWidget(parent)
    {
        return;
    }

    void setLabel(const int index, QLabel *label)
    {
        this->tabBar()->setTabButton(index, QTabBar::LeftSide, label);
    }
};

/***********************************************************************
 * top level window for node info
 **********************************************************************/
class RemoteNodesWindow : public QWidget
{
    Q_OBJECT
public:
    RemoteNodesWindow(QWidget *parent):
        QWidget(parent),
        _startMapper(new QSignalMapper(this)),
        _stopMapper(new QSignalMapper(this))
    {
        auto layout = new QVBoxLayout(this);
        auto table = makeRemoteNodesTable(this);
        layout->addWidget(table);

        _tabs = new RemoteNodeInfoTabs(this);
        _tabs->addTab(makeSystemInfoTree(_tabs), tr("System Info"));
        _tabs->addTab(makePluginRegistryTree(_tabs), tr("Plugin Registry"));
        _tabs->addTab(makePluginModuleTree(_tabs), tr("Plugin Modules"));
        layout->addWidget(_tabs, 1);

        //connect mappers
        connect(
            _startMapper, SIGNAL(mapped(const int)),
            this, SLOT(start(const int)));
        connect(
            _stopMapper, SIGNAL(mapped(const int)),
            this, SLOT(stop(const int)));

        //connect handlers for node info so the tabs update
        for (int i = 0; i < _tabs->count(); i++)
        {
            connect(
                table, SIGNAL(nodeInfoRequest(const Pothos::RemoteNode &)),
                _tabs->widget(i), SLOT(handeNodeInfoRequest(const Pothos::RemoteNode &)));

            connect(_tabs->widget(i), SIGNAL(startLoad(void)), _startMapper, SLOT(map(void)));
            _startMapper->setMapping(_tabs->widget(i), i);

            connect(_tabs->widget(i), SIGNAL(stopLoad(void)), _stopMapper, SLOT(map(void)));
            _stopMapper->setMapping(_tabs->widget(i), i);
        }
    }

private slots:
    void start(const int index)
    {
        auto label = new QLabel(_tabs);
        auto movie = new QMovie(makeIconPath("loading.gif"), QByteArray(), label);
        label->setMovie(movie);
        movie->start();
        _tabs->setLabel(index, label);
    }

    void stop(const int index)
    {
        _tabs->setLabel(index, new QLabel(_tabs));
    }
private:
    RemoteNodeInfoTabs *_tabs;
    QSignalMapper *_startMapper;
    QSignalMapper *_stopMapper;
};

QWidget *makeRemoteNodesWindow(QWidget *parent)
{
    return new RemoteNodesWindow(parent);
}

#include "RemoteNodesWindow.moc"
