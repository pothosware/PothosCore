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
#include <algorithm> //std::set_difference

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

        //save pre-edit settings
        _originalKeyPairs = _conn->getSigSlotPairs();
    }

    //remove button
    {
        _removeButton = new QPushButton(makeIconFromTheme("list-remove"), tr("Remove connection"), this);
        connect(_removeButton, SIGNAL(pressed(void)), this, SLOT(handleRemoveConnection(void)));
        layout->addWidget(_removeButton);
    }

    connect(_conn, SIGNAL(destroyed(QObject*)), this, SLOT(handleConnectionDestroyed(QObject*)));
    this->handleItemSelectionChanged(); //init state
    this->populateConnectionsList(); //init state
}

void ConnectionPropertiesPanel::handleConnectionDestroyed(QObject *)
{
    this->deleteLater();
}

void ConnectionPropertiesPanel::handleCancel(void)
{
    //restore original settings
    _conn->setSigSlotPairs(_originalKeyPairs);
}

template <typename T>
T mySetDifference(const T &a, const T &b)
{
    T aSort = a; std::sort(aSort.begin(), aSort.end());
    T bSort = b; std::sort(bSort.begin(), bSort.end());
    T v(std::max(a.size(), b.size()));
    auto it = std::set_difference(aSort.begin(), aSort.end(), bSort.begin(), bSort.end(), v.begin());
    v.resize(it-v.begin());
    return v;
}

void ConnectionPropertiesPanel::handleCommit(void)
{
    const auto createdPairs = mySetDifference(_conn->getSigSlotPairs(), _originalKeyPairs);
    const auto removedPairs = mySetDifference(_originalKeyPairs, _conn->getSigSlotPairs());

    //description of the change
    QString desc;
    if (createdPairs.empty() and removedPairs.empty()) return this->handleCancel();
    else if (createdPairs.empty() and removedPairs.size() == 1)
    {
        desc = tr("Removed %1->%2").arg(removedPairs.at(0).first).arg(removedPairs.at(0).second);
    }
    else if (createdPairs.size() == 1 and removedPairs.empty())
    {
        desc = tr("Created %1->%2").arg(createdPairs.at(0).first).arg(createdPairs.at(0).second);
    }
    else
    {
        desc = tr("Changed signals->slots");
    }

    //emit a new graph state event
    emit this->stateChanged(GraphState("document-properties", desc));
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
            _conn->addSigSlotPair(pair);
            this->populateConnectionsList();
        }
    }
}

void ConnectionPropertiesPanel::populateConnectionsList(void)
{
    _connectionsListWidget->clear();
    _connItemToKeyPair.clear();
    for (const auto &pair : _conn->getSigSlotPairs())
    {
        auto item = new QTreeWidgetItem(_connectionsListWidget, QStringList(QString("%1 -> %2")
            .arg(pair.first).arg(pair.second)));
        _connItemToKeyPair[item] = pair;
        _connectionsListWidget->addTopLevelItem(item);
    }
}

void ConnectionPropertiesPanel::handleRemoveConnection(void)
{
    for (auto item : _connectionsListWidget->selectedItems())
    {
        _conn->removeSigSlotPair(_connItemToKeyPair.at(item));
        delete item;
        this->populateConnectionsList();
    }
}
