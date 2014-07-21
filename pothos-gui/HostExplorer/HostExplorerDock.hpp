// Copyright (c) 2013-2013 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QDockWidget>
#include <QStringList>

class HostSelectionTable;
class QSignalMapper;
class QTabWidget;

//! top level dock widget for dealing with hosts
class HostExplorerDock : public QDockWidget
{
    Q_OBJECT
public:
    HostExplorerDock(QWidget *parent);

    //! Get a list of available host uris
    QStringList hostUriList(void) const;

signals:

    //! Emitted when the list of host uris changes
    void hostUriListChanged(void);

private slots:
    void start(const int index);

    void stop(const int index);

private:
    HostSelectionTable *_table;
    QTabWidget *_tabs;
    QSignalMapper *_startMapper;
    QSignalMapper *_stopMapper;
};
