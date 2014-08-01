// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PropertiesPanel/PropertiesPanelDock.hpp"
#include "PropertiesPanel/BlockPropertiesPanel.hpp"
#include "PropertiesPanel/ConnectionPropertiesPanel.hpp"
#include <GraphObjects/GraphBlock.hpp>
#include <GraphObjects/GraphBreaker.hpp>
#include <GraphObjects/GraphConnection.hpp>

PropertiesPanelDock::PropertiesPanelDock(QWidget *parent):
    QDockWidget(parent),
    _propertiesPanel(nullptr)
{
    this->setObjectName("PropertiesPanelDock");
    this->setWindowTitle(tr("Properties Panel"));
    this->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
}

void PropertiesPanelDock::handleGraphModifyProperties(GraphObject *obj)
{
    //clear old panel
    if (_propertiesPanel)
    {
        emit this->resetPanel();
        delete _propertiesPanel;
    }

    //extract the graph object
    auto block = dynamic_cast<GraphBlock *>(obj);
    auto breaker = dynamic_cast<GraphBreaker *>(obj);
    auto connection = dynamic_cast<GraphConnection *>(obj);

    if (block != nullptr) _propertiesPanel = new BlockPropertiesPanel(block, this);
    else if (breaker != nullptr) return; //TODO
    else if (connection != nullptr) _propertiesPanel = new ConnectionPropertiesPanel(connection, this);
    else return;

    //connect panel signals and slots into dock events
    connect(_propertiesPanel, SIGNAL(destroyed(QObject*)), this, SLOT(handlePanelDestroyed(QObject *)));
    connect(this, SIGNAL(resetPanel(void)), _propertiesPanel, SLOT(handleReset(void)));

    //set the widget and make the entire dock visible
    this->setWidget(_propertiesPanel);
    this->show();
    this->raise();
}

void PropertiesPanelDock::handlePanelDestroyed(QObject *)
{
    this->hide();
}
