// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include "GraphEditor/GraphState.hpp"
#include <QWidget>
#include <QPointer>
#include <QString>
#include <map>

class GraphConnection;
class QPushButton;
class QTreeWidgetItem;
class QTreeWidget;

class ConnectionPropertiesPanel : public QWidget
{
    Q_OBJECT
public:
    ConnectionPropertiesPanel(GraphConnection *conn, QWidget *parent);

signals:
    void stateChanged(const GraphState &);

public slots:
    void handleCancel(void);
    void handleCommit(void);

private slots:

    void handleConnectionDestroyed(QObject *);
    void handleItemSelectionChanged(void);
    void handleCreateConnection(void);
    void handleRemoveConnection(void);

private:
    QPointer<GraphConnection> _conn;
    const bool _isSlot;
    const bool _isSignal;
    QPushButton *_connectButton;
    QPushButton *_removeButton;
    QTreeWidget *_inputListWidget;
    QTreeWidget *_outputListWidget;
    QTreeWidget *_connectionsListWidget;
    void populateConnectionsList(void);
    std::map<QTreeWidgetItem *, QString> _inputItemToKey;
    std::map<QTreeWidgetItem *, QString> _outputItemToKey;
    std::map<QTreeWidgetItem *, std::pair<QString, QString>> _connItemToKeyPair;
    std::vector<std::pair<QString, QString>> _originalKeyPairs;
};
