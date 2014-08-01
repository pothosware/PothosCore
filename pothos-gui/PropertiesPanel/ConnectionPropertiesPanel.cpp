// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGuiUtils.hpp" //make icon theme
#include "PropertiesPanel/ConnectionPropertiesPanel.hpp"
#include "GraphObjects/GraphConnection.hpp"
#include "GraphObjects/GraphBlock.hpp"
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QScrollArea>
#include <QLabel>

ConnectionPropertiesPanel::ConnectionPropertiesPanel(GraphConnection *conn, QWidget *parent):
    QWidget(parent),
    _conn(conn)
{
    //master layout for this widget
    auto layout = new QVBoxLayout(this);

    //create a scroller and a layout
    auto scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setWidget(new QWidget(scroll));
    auto scrollLayout = new QVBoxLayout();
    scroll->widget()->setLayout(scrollLayout);
    layout->addWidget(scroll);

    auto inputEp = _conn->getInputEndpoint();
    auto isSlot = (inputEp.getConnectableAttrs().direction == GRAPH_CONN_SLOT);
    auto outputEp = _conn->getOutputEndpoint();
    auto isSignal = (outputEp.getConnectableAttrs().direction == GRAPH_CONN_SIGNAL);

    //title
    {
        auto label = new QLabel(tr("<h1>%1</h1>").arg(tr("Connection")), this);
        label->setAlignment(Qt::AlignCenter);
        scrollLayout->addWidget(label);
    }

    //signal/slots selection boxes
    {
        auto listWidgetLayout = new QHBoxLayout();
        scrollLayout->addLayout(listWidgetLayout);

        //output list
        auto outputListWidget = new QTreeWidget(this);
        listWidgetLayout->addWidget(outputListWidget);
        outputListWidget->setColumnCount(1);
        outputListWidget->setHeaderLabels(QStringList(QString("%1[%2]").arg(outputEp.getObj()->getId(), outputEp.getKey().id)));

        //populate output list
        auto outputBlock = dynamic_cast<GraphBlock *>(outputEp.getObj().data());
        if (isSignal and outputBlock != nullptr)
        {
            for (const auto &port : outputBlock->getSignalPorts())
            {
                auto item = new QTreeWidgetItem(outputListWidget, QStringList(port.getName()));
                outputListWidget->addTopLevelItem(item);
            }
        }
        outputListWidget->resizeColumnToContents(0);

        //input list
        auto inputListWidget = new QTreeWidget(this);
        listWidgetLayout->addWidget(inputListWidget);
        inputListWidget->setColumnCount(1);
        inputListWidget->setHeaderLabels(QStringList(QString("%1[%2]").arg(inputEp.getObj()->getId(), inputEp.getKey().id)));

        //populate input list
        auto inputBlock = dynamic_cast<GraphBlock *>(inputEp.getObj().data());
        if (isSlot and inputBlock != nullptr)
        {
            for (const auto &port : inputBlock->getSlotPorts())
            {
                auto item = new QTreeWidgetItem(inputListWidget, QStringList(port.getName()));
                inputListWidget->addTopLevelItem(item);
            }
        }
        inputListWidget->resizeColumnToContents(0);
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
