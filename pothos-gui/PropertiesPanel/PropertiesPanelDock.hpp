// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QDockWidget>
#include <QPointer>

class GraphObject;
class QScrollArea;
class QPushButton;

class PropertiesPanelDock : public QDockWidget
{
    Q_OBJECT
public:
    PropertiesPanelDock(QWidget *parent);

signals:
    void replacePanel(void);

private slots:

    void handleGraphModifyProperties(GraphObject *obj);

    void handlePanelDestroyed(QObject *);

    void handleDeletePanel(void);

private:
    QPointer<GraphObject> _currentGraphObject;
    QPointer<QWidget> _propertiesPanel;
    QScrollArea *_scroll;
    QPushButton *_commitButton;
    QPushButton *_cancelButton;
};
