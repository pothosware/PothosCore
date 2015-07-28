// Copyright (c) 2013-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "MessageWindow/MessageWindowDock.hpp"
#include "MessageWindow/LoggerDisplay.hpp"
#include <QTabWidget>
#include <QTabBar>

MessageWindowDock::MessageWindowDock(QWidget *parent):
    QDockWidget(parent),
    _tabs(new QTabWidget(this))
{
    this->setObjectName("MessageWindowDock");
    this->setWindowTitle(tr("Message Window"));
    this->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    this->setWidget(_tabs);

    _tabs->tabBar()->hide(); //hide tab bar until we use it
    _tabs->setMovable(true);
    _tabs->setUsesScrollButtons(true);
    _tabs->setTabPosition(QTabWidget::West);
    _tabs->addTab(new LoggerDisplay(this), "");
}
