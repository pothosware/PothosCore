// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PropertiesPanel/PropertiesPanelDock.hpp"
#include "PropertiesPanel/BlockPropertiesPanel.hpp"
#include <GraphObjects/GraphBlock.hpp>

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
    auto block = dynamic_cast<GraphBlock *>(obj);
    if (block != nullptr)
    {
        if (_propertiesPanel)
        {
            auto _blockPropertiesPanel = dynamic_cast<BlockPropertiesPanel *>(_propertiesPanel.data());
            if (_blockPropertiesPanel != nullptr) _blockPropertiesPanel->reset();
            delete _propertiesPanel;
        }
        _propertiesPanel = new BlockPropertiesPanel(block, this);
        connect(_propertiesPanel, SIGNAL(destroyed(QObject*)), this, SLOT(handlePanelDestroyed(QObject *)));
        this->setWidget(_propertiesPanel);
        this->show();
        this->raise();
    }
}

void PropertiesPanelDock::handlePanelDestroyed(QObject *)
{
    this->hide();
}
