// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGuiUtils.hpp" //make icon theme
#include "PropertiesPanel/PropertiesPanelDock.hpp"
#include "PropertiesPanel/BlockPropertiesPanel.hpp"
#include "PropertiesPanel/ConnectionPropertiesPanel.hpp"
#include "GraphObjects/GraphBlock.hpp"
#include "GraphObjects/GraphBreaker.hpp"
#include "GraphObjects/GraphConnection.hpp"
#include "GraphEditor/GraphDraw.hpp"
#include "GraphEditor/GraphEditor.hpp"
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>

PropertiesPanelDock::PropertiesPanelDock(QWidget *parent):
    QDockWidget(parent),
    _propertiesPanel(nullptr),
    _scroll(new QScrollArea(this)),
    _commitButton(nullptr),
    _cancelButton(nullptr)
{
    this->setObjectName("PropertiesPanelDock");
    this->setWindowTitle(tr("Properties Panel"));
    this->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    this->setWidget(new QWidget(this));

    //master layout for this widget
    auto layout = new QVBoxLayout(this->widget());

    //setup the scroller
    _scroll->setWidgetResizable(true);
    layout->addWidget(_scroll);

    //setup the buttons
    {
        auto buttonLayout = new QHBoxLayout();
        layout->addLayout(buttonLayout);
        _commitButton = new QPushButton(makeIconFromTheme("dialog-ok-apply"), tr("Commit"), this);
        connect(_commitButton, SIGNAL(pressed(void)), this, SLOT(handleDeletePanel(void)));
        buttonLayout->addWidget(_commitButton);
        _cancelButton = new QPushButton(makeIconFromTheme("dialog-cancel"), tr("Cancel"), this);
        connect(_cancelButton, SIGNAL(pressed(void)), this, SLOT(handleDeletePanel(void)));
        buttonLayout->addWidget(_cancelButton);
    }
}

void PropertiesPanelDock::handleGraphModifyProperties(GraphObject *obj)
{
    //clear old panel
    if (_propertiesPanel)
    {
        if (_currentGraphObject) emit this->resetPanel();
        delete _propertiesPanel;
    }

    //extract the graph object
    auto block = dynamic_cast<GraphBlock *>(obj);
    auto breaker = dynamic_cast<GraphBreaker *>(obj);
    auto connection = dynamic_cast<GraphConnection *>(obj);

    if (block != nullptr) _propertiesPanel = new BlockPropertiesPanel(block, this);
    else if (breaker != nullptr) return; //TODO
    else if (connection != nullptr and connection->isSignalOrSlot()) _propertiesPanel = new ConnectionPropertiesPanel(connection, this);
    else return;

    //connect panel signals and slots into dock events
    connect(_propertiesPanel, SIGNAL(destroyed(QObject*)), this, SLOT(handlePanelDestroyed(QObject *)));
    connect(this, SIGNAL(resetPanel(void)), _propertiesPanel, SLOT(handleCancel(void)));
    connect(_commitButton, SIGNAL(pressed(void)), _propertiesPanel, SLOT(handleCommit(void)));
    connect(_cancelButton, SIGNAL(pressed(void)), _propertiesPanel, SLOT(handleCancel(void)));

    //connect state change to the graph editor
    auto draw = dynamic_cast<GraphDraw *>(obj->parent());
    auto editor = draw->getGraphEditor();
    connect(_propertiesPanel, SIGNAL(stateChanged(const GraphState &)), editor, SLOT(handleStateChange(const GraphState &)));

    //set the widget and make the entire dock visible
    _currentGraphObject = obj;
    _scroll->setWidget(_propertiesPanel);
    this->show();
    this->raise();
}

void PropertiesPanelDock::handlePanelDestroyed(QObject *)
{
    this->hide();
}

void PropertiesPanelDock::handleDeletePanel(void)
{
    if (_propertiesPanel) _propertiesPanel->deleteLater();
}
