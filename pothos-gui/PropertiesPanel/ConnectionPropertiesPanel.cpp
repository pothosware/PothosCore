// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGuiUtils.hpp" //make icon theme
#include "PropertiesPanel/ConnectionPropertiesPanel.hpp"
#include "GraphObjects/GraphConnection.hpp"
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QLabel>

ConnectionPropertiesPanel::ConnectionPropertiesPanel(GraphConnection *conn, QWidget *parent):
    QWidget(parent),
    _conn(conn)
{
    auto layout = new QVBoxLayout(this);

    auto inputEp = _conn->getInputEndpoint();
    auto isSlot = (inputEp.getConnectableAttrs().direction == GRAPH_CONN_SLOT);
    auto outputEp = _conn->getOutputEndpoint();
    auto isSignal = (inputEp.getConnectableAttrs().direction == GRAPH_CONN_SIGNAL);

    //title
    {
        auto label = new QLabel(tr("<h1>%1</h1>").arg(tr("Connection")), this);
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);
    }

    //signal/slots selection boxes
    {
        auto listWidgetLayout = new QHBoxLayout();
        layout->addLayout(listWidgetLayout);

        auto outputListWidget = new QTreeWidget(this);
        listWidgetLayout->addWidget(outputListWidget);
        outputListWidget->setColumnCount(1);
        outputListWidget->setHeaderLabels(QStringList(QString("%1[%2]").arg(outputEp.getObj()->getId(), outputEp.getKey().id)));

        auto inputListWidget = new QTreeWidget(this);
        listWidgetLayout->addWidget(inputListWidget);
        inputListWidget->setColumnCount(1);
        inputListWidget->setHeaderLabels(QStringList(QString("%1[%2]").arg(inputEp.getObj()->getId(), inputEp.getKey().id)));
    }

    //buttons
    {
        auto buttonLayout = new QHBoxLayout();
        layout->addLayout(buttonLayout);
        auto commitButton = new QPushButton(makeIconFromTheme("dialog-ok-apply"), tr("Commit"), this);
        connect(commitButton, SIGNAL(pressed(void)), this, SLOT(handleCommitButton(void)));
        buttonLayout->addWidget(commitButton);
        auto cancelButton = new QPushButton(makeIconFromTheme("dialog-cancel"), tr("Cancel"), this);
        connect(cancelButton, SIGNAL(pressed(void)), this, SLOT(handleCancelButton(void)));
        buttonLayout->addWidget(cancelButton);
    }

    connect(_conn, SIGNAL(destroyed(QObject*)), this, SLOT(handleConnectionDestroyed(QObject*)));
}

void ConnectionPropertiesPanel::handleConnectionDestroyed(QObject *)
{
    this->deleteLater();
}

void ConnectionPropertiesPanel::handleCancelButton(void)
{
    this->handleReset();
    this->deleteLater();
}

void ConnectionPropertiesPanel::handleCommitButton(void)
{
    
}

void ConnectionPropertiesPanel::handleReset(void)
{
    
}
