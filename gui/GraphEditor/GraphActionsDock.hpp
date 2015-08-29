// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QDockWidget>

class QStackedWidget;

//! A top level dock widget for showing the active actions panel
class GraphActionsDock : public QDockWidget
{
    Q_OBJECT
public:
    GraphActionsDock(QWidget *parent);

    void setActiveWidget(QWidget *widget);

private:
    QStackedWidget *_stack;
};
