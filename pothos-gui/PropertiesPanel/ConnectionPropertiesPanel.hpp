// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QWidget>
#include <QPointer>

class GraphConnection;

class ConnectionPropertiesPanel : public QWidget
{
    Q_OBJECT
public:
    ConnectionPropertiesPanel(GraphConnection *conn, QWidget *parent);

public slots:
    //! reset all settings to their original (pre-edit) values
    void handleReset(void);

private slots:

    void handleConnectionDestroyed(QObject *);

    void handleCancelButton(void);

    void handleCommitButton(void);

private:
    QPointer<GraphConnection> _conn;
};
