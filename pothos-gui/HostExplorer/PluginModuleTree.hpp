// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QTreeWidget>
#include <QFutureWatcher>
#include <map>
#include <vector>
#include <string>

//! tree widget display for a host's loaded modules
class PluginModuleTree : public QTreeWidget
{
    Q_OBJECT
public:
    PluginModuleTree(QWidget *parent);

    typedef std::map<std::string, std::vector<std::string>> ModMapType;

signals:
    void startLoad(void);
    void stopLoad(void);

public slots:
    void handeInfoRequest(const std::string &uriStr);

private slots:

    void handleWatcherDone(void);

private:
    QFutureWatcher<ModMapType> *_watcher;
};
