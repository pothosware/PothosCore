// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QDockWidget>

class QTabWidget;

//! top level dock for message/logger text displays
class MessageWindowDock : public QDockWidget
{
    Q_OBJECT
public:
    MessageWindowDock(QWidget *parent);

private:
    QTabWidget *_tabs;
};
