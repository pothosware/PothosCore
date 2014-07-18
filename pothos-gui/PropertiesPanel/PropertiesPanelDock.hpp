// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QDockWidget>
#include <QPointer>

class GraphObject;

class PropertiesPanelDock : public QDockWidget
{
    Q_OBJECT
public:
    PropertiesPanelDock(QWidget *parent);

private slots:

    void handleGraphModifyProperties(GraphObject *obj);

    void handlePanelDestroyed(QObject *);

private:
    QPointer<QWidget> _propertiesPanel;
};
