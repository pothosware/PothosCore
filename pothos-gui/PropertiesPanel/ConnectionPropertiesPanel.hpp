// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QWidget>
#include <QPointer>
#include <QString>
#include <set>
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
    std::map<QTreeWidgetItem *, QString> _inputItemToKey;
    std::map<QTreeWidgetItem *, QString> _outputItemToKey;
    std::map<QTreeWidgetItem *, std::pair<QString, QString>> _connItemToKeyPair;
    std::set<std::pair<QString, QString>> getKeyPairs(void) const
    {
        std::set<std::pair<QString, QString>> out;
        for (const auto &it : _connItemToKeyPair) out.insert(it.second);
        return out;
    }
};
