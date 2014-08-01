// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGuiUtils.hpp" //make icon from theme
#include "PropertiesPanel/ConnectionPropertiesPanel.hpp"
#include "GraphObjects/GraphConnection.hpp"
#include "GraphObjects/GraphBlock.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QPushButton>
#include <QLabel>

ConnectionPropertiesPanel::ConnectionPropertiesPanel(GraphConnection *conn, QWidget *parent):
    QWidget(parent),
    _conn(conn),
    _isSlot(_conn->getInputEndpoint().getConnectableAttrs().direction == GRAPH_CONN_SLOT),
    _isSignal(_conn->getOutputEndpoint().getConnectableAttrs().direction == GRAPH_CONN_SIGNAL),
    _connectButton(nullptr),
    _removeButton(nullptr),
    _inputListWidget(nullptr),
    _outputListWidget(nullptr),
    _connectionsListWidget(nullptr)
{
    //master layout for this widget
    auto layout = new QVBoxLayout(this);

    auto inputEp = _conn->getInputEndpoint();
    auto outputEp = _conn->getOutputEndpoint();

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
        _outputListWidget = new QTreeWidget(this);
        listWidgetLayout->addWidget(_outputListWidget);
        _outputListWidget->setColumnCount(1);
        _outputListWidget->setHeaderLabels(QStringList(QString("%1[%2]").arg(outputEp.getObj()->getId(), outputEp.getKey().id)));
        connect(_outputListWidget, SIGNAL(itemSelectionChanged(void)), this, SLOT(handleItemSelectionChanged(void)));

        //populate output list
        auto outputBlock = dynamic_cast<GraphBlock *>(outputEp.getObj().data());
        if (_isSignal and outputBlock != nullptr)
        {
            for (const auto &port : outputBlock->getSignalPorts())
            {
                auto item = new QTreeWidgetItem(_outputListWidget, QStringList(port.getName()));
                _outputItemToKey[item] = port.getKey();
                _outputListWidget->addTopLevelItem(item);
            }
        }
        _outputListWidget->resizeColumnToContents(0);

        //input list
        _inputListWidget = new QTreeWidget(this);
        listWidgetLayout->addWidget(_inputListWidget);
        _inputListWidget->setColumnCount(1);
        _inputListWidget->setHeaderLabels(QStringList(QString("%1[%2]").arg(inputEp.getObj()->getId(), inputEp.getKey().id)));
        connect(_inputListWidget, SIGNAL(itemSelectionChanged(void)), this, SLOT(handleItemSelectionChanged(void)));

        //populate input list
        auto inputBlock = dynamic_cast<GraphBlock *>(inputEp.getObj().data());
        if (_isSlot and inputBlock != nullptr)
        {
            for (const auto &port : inputBlock->getSlotPorts())
            {
                auto item = new QTreeWidgetItem(_inputListWidget, QStringList(port.getName()));
                _inputItemToKey[item] = port.getKey();
                _inputListWidget->addTopLevelItem(item);
            }
        }
        _inputListWidget->resizeColumnToContents(0);
    }

    //connect button
    {
        _connectButton = new QPushButton(makeIconFromTheme("list-add"), tr("Create connection"), this);
        connect(_connectButton, SIGNAL(pressed(void)), this, SLOT(handleCreateConnection(void)));
        layout->addWidget(_connectButton);
    }

    //existing connections
    {
        _connectionsListWidget = new QTreeWidget(this);
        layout->addWidget(_connectionsListWidget);
        _connectionsListWidget->setColumnCount(1);
        _connectionsListWidget->setHeaderLabels(QStringList(tr("Signal slot connections")));
        connect(_connectionsListWidget, SIGNAL(itemSelectionChanged(void)), this, SLOT(handleItemSelectionChanged(void)));
    }

    //remove button
    {
        _removeButton = new QPushButton(makeIconFromTheme("list-remove"), tr("Remove connection"), this);
        connect(_removeButton, SIGNAL(pressed(void)), this, SLOT(handleRemoveConnection(void)));
        layout->addWidget(_removeButton);
    }

    connect(_conn, SIGNAL(destroyed(QObject*)), this, SLOT(handleConnectionDestroyed(QObject*)));
    this->handleItemSelectionChanged(); //init state
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

void ConnectionPropertiesPanel::handleItemSelectionChanged(void)
{
    auto signalItemsSelected = _outputListWidget->selectedItems();
    auto slotItemsSelected = _inputListWidget->selectedItems();
    _connectButton->setEnabled(not signalItemsSelected.isEmpty() and not slotItemsSelected.isEmpty());
    _removeButton->setEnabled(not _connectionsListWidget->selectedItems().isEmpty());
}

void ConnectionPropertiesPanel::handleCreateConnection(void)
{
    for (auto signalItem : _outputListWidget->selectedItems())
    {
        signalItem->setSelected(false);
        for (auto slotItem : _inputListWidget->selectedItems())
        {
            slotItem->setSelected(false);
            auto pair = std::make_pair(
                _outputItemToKey.at(signalItem),
                _inputItemToKey.at(slotItem));
            if (this->getKeyPairs().count(pair) != 0) continue;
            auto item = new QTreeWidgetItem(_connectionsListWidget, QStringList(QString("%1 -> %2")
                .arg(signalItem->text(0))
                .arg(slotItem->text(0))));
            _connItemToKeyPair[item] = pair;
            _connectionsListWidget->addTopLevelItem(item);
        }
    }
}

void ConnectionPropertiesPanel::handleRemoveConnection(void)
{
    for (auto item : _connectionsListWidget->selectedItems())
    {
        _connItemToKeyPair.erase(item);
        delete item;
    }
}
