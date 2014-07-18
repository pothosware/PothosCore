// Copyright (c) 2013-2013 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <QDockWidget>

class QSignalMapper;
class HostInfoTabWidget;

//! top level dock widget for dealing with hosts
class HostExplorerDock : public QDockWidget
{
    Q_OBJECT
public:
    HostExplorerDock(QWidget *parent);

private slots:
    void start(const int index);

    void stop(const int index);

private:
    HostInfoTabWidget *_tabs;
    QSignalMapper *_startMapper;
    QSignalMapper *_stopMapper;
};
