// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QFutureWatcher>
#include <map>
#include <string>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>

//! Get a block description given the block registry path
Poco::JSON::Object::Ptr getBlockDescFromPath(const std::string &path);

class BlockCache : public QObject
{
    Q_OBJECT
public:

    BlockCache(QObject *parent);

signals:
    void blockDescUpdate(const Poco::JSON::Array::Ptr &);
    void blockDescReady(void);

public slots:
    void handleUpdate(void);

private slots:
    void handleWatcherFinished(void);

    void handleWatcherDone(const int which);

private:
    QStringList _allRemoteNodeUris;
    QFutureWatcher<Poco::JSON::Array::Ptr> *_watcher;

    //storage structures
    std::map<QString, Poco::JSON::Array::Ptr> _uriToBlockDescs;
    std::map<std::string, Poco::JSON::Object::Ptr> &_registryPathToBlockDesc;
    Poco::JSON::Array::Ptr _superSetBlockDescs;
};
