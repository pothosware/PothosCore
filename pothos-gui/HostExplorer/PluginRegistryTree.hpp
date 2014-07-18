// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <QTreeWidget>
#include <QFutureWatcher>
#include <Pothos/Plugin/Registry.hpp>
#include <string>

//! tree widget display for a host's plugin registry
class PluginRegistryTree : public QTreeWidget
{
    Q_OBJECT
public:
    PluginRegistryTree(QWidget *parent);

signals:
    void startLoad(void);
    void stopLoad(void);

private slots:

    void handeNodeInfoRequest(const std::string &uriStr);

    void handleWatcherDone(void);

private:
    QFutureWatcher<Pothos::PluginRegistryInfoDump> *_watcher;
};
