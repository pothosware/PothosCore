// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PropertiesPanel/ConnectionPropertiesPanel.hpp"
#include "GraphObjects/GraphConnection.hpp"
#include "GraphObjects/GraphBlock.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QLabel>

ConnectionPropertiesPanel::ConnectionPropertiesPanel(GraphConnection *conn, QWidget *parent):
    QWidget(parent),
    _conn(conn)
{
    //master layout for this widget
    auto layout = new QVBoxLayout(this);

    auto inputEp = _conn->getInputEndpoint();
    auto isSlot = (inputEp.getConnectableAttrs().direction == GRAPH_CONN_SLOT);
    auto outputEp = _conn->getOutputEndpoint();
    auto isSignal = (outputEp.getConnectableAttrs().direction == GRAPH_CONN_SIGNAL);

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

    connect(_conn, SIGNAL(destroyed(QObject*)), this, SLOT(handleConnectionDestroyed(QObject*)));
}

void ConnectionPropertiesPanel::handleConnectionDestroyed(QObject *)
{
}

void ConnectionPropertiesPanel::handleCancel(void)
{
}

void ConnectionPropertiesPanel::handleCommit(void)
{
    
}
