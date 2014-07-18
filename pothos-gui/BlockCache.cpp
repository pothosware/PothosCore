// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGuiUtils.hpp" //get hosts list
#include "BlockCache.hpp"
#include "GraphObjects/GraphBlock.hpp"
#include <Pothos/Remote.hpp>
#include <Pothos/Proxy.hpp>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/Logger.h>
#include <Poco/SingletonHolder.h>
#include <iostream>
#include <map>

/***********************************************************************
 * lookup cache
 **********************************************************************/
static std::map<std::string, Poco::JSON::Object::Ptr> &getRegistryPathToBlockDesc(void)
{
    static Poco::SingletonHolder<std::map<std::string, Poco::JSON::Object::Ptr>> sh;
    return *sh.get();
}

Poco::JSON::Object::Ptr getBlockDescFromPath(const std::string &path)
{
    //look in the cache
    auto it = getRegistryPathToBlockDesc().find(path);
    if (it != getRegistryPathToBlockDesc().end()) return it->second;

    //search all of the nodes
    for (const auto &uri : getRemoteNodeUris())
    {
        try
        {
            auto client = Pothos::RemoteClient(uri.toStdString());
            auto env = client.makeEnvironment("managed");
            auto DocUtils = env->findProxy("Pothos/Util/DocUtils");
            return DocUtils.call<Poco::JSON::Object::Ptr>("dumpJsonAt", path);
        }
        catch (const Pothos::Exception &)
        {
            //pass
        }
    }

    return Poco::JSON::Object::Ptr();
}

/***********************************************************************
 * Query JSON docs from node
 **********************************************************************/
static Poco::JSON::Array::Ptr queryBlockDescs(const QString &uri)
{
    try
    {
        auto client = Pothos::RemoteClient(uri.toStdString());
        auto env = client.makeEnvironment("managed");
        return env->findProxy("Pothos/Util/DocUtils").call<Poco::JSON::Array::Ptr>("dumpJson");
    }
    catch (const Pothos::Exception &ex)
    {
        poco_warning_f2(Poco::Logger::get("PothosGui.BlockCache"), "Failed to query JSON Docs from %s - %s", uri.toStdString(), ex.displayText());
    }

    return Poco::JSON::Array::Ptr(); //empty JSON array
}

/***********************************************************************
 * Block Cache impl
 **********************************************************************/
BlockCache::BlockCache(QObject *parent):
    QObject(parent),
    _watcher(new QFutureWatcher<Poco::JSON::Array::Ptr>(this)),
    _registryPathToBlockDesc(getRegistryPathToBlockDesc())
{
    connect(_watcher, SIGNAL(resultReadyAt(int)), this, SLOT(handleWatcherDone(int)));
    connect(_watcher, SIGNAL(finished()), this, SLOT(handleWatcherFinished()));
}

void BlockCache::handleUpdate(void)
{
    if (_watcher->isRunning()) return;

    //nodeKeys cannot be a temporary because QtConcurrent will reference them
    _allRemoteNodeUris = getRemoteNodeUris();
    _watcher->setFuture(QtConcurrent::mapped(_allRemoteNodeUris, &queryBlockDescs));
}

void BlockCache::handleWatcherFinished(void)
{
    //remove old nodes
    std::map<QString, Poco::JSON::Array::Ptr> newMap;
    for (const auto &uri : _allRemoteNodeUris) newMap[uri] = _uriToBlockDescs[uri];
    _uriToBlockDescs = newMap;

    //map paths to block descs
    _registryPathToBlockDesc.clear();
    for (const auto &pair : _uriToBlockDescs)
    {
        if (not pair.second) continue;
        for (const auto &blockDescObj : *pair.second)
        {
            const auto blockDesc = blockDescObj.extract<Poco::JSON::Object::Ptr>();
            const auto path = blockDesc->get("path").extract<std::string>();
            _registryPathToBlockDesc[path] = blockDesc;
        }
    }

    //make a master block desc list
    _superSetBlockDescs = new Poco::JSON::Array();
    for (const auto &pair : _registryPathToBlockDesc)
    {
        _superSetBlockDescs->add(pair.second);
    }

    //let the subscribers know
    emit this->blockDescReady();
    emit this->blockDescUpdate(_superSetBlockDescs);
}

void BlockCache::handleWatcherDone(const int which)
{
    _uriToBlockDescs[_allRemoteNodeUris[which]] = _watcher->resultAt(which);
}
