// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "GraphEditor/GraphActionsDock.hpp"
#include <QStackedWidget>

GraphActionsDock::GraphActionsDock(QWidget *parent):
    QDockWidget(parent),
    _stack(new QStackedWidget(this))
{
    this->setObjectName("GraphActionsDock");
    this->setWindowTitle(tr("Graph Actions"));
    this->setWidget(_stack);
}

void GraphActionsDock::setActiveWidget(QWidget *widget)
{
    _stack->addWidget(widget);
    _stack->setCurrentWidget(widget);
}
