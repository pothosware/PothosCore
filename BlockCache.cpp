// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGui.hpp"
#include <Pothos/Remote.hpp>
#include <Pothos/Proxy.hpp>
#include "GraphObjects/GraphBlock.hpp"
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include <Poco/JSON/Parser.h>
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
    for (const auto &key : Pothos::RemoteNode::listRegistryKeys())
    {
        try
        {
            auto node = Pothos::RemoteNode::fromKey(key);
            auto client = node.makeClient("json");
            auto env = client.makeEnvironment("managed");
            auto plugin = env->findProxy("Pothos/PluginRegistry").callProxy("get", "/blocks/docs"+path);
            auto obj = plugin.call<Pothos::Object>("getObject");
            Poco::JSON::Parser p; p.parse(obj.convert<std::string>());
            return p.getHandler()->asVar().extract<Poco::JSON::Object::Ptr>();
        }
        catch (const Pothos::Exception &ex)
        {
            std::cerr << ex.displayText() << std::endl;
            continue;
        }
        catch (...)
        {
            continue;
        }
    }

    return Poco::JSON::Object::Ptr();
}

/***********************************************************************
 * Query JSON docs from node
 **********************************************************************/
static Poco::JSON::Array::Ptr queryBlockDescs(const std::string &nodeKey)
{
    try
    {
        auto node = Pothos::RemoteNode::fromKey(nodeKey);
        auto env = node.makeClient("json").makeEnvironment("managed");
        const auto json = env->findProxy("Pothos/Gui/DocUtils").call<std::string>("dumpJson");

        Poco::JSON::Parser p; p.parse(json);
        return p.getHandler()->asVar().extract<Poco::JSON::Array::Ptr>();
    }
    catch (const Pothos::Exception &ex)
    {
        const auto uri = Pothos::RemoteNode::fromKey(nodeKey).getUri();
        poco_error_f2(Poco::Logger::get("PothosGui.BlockCache"), "Failed to query JSON Docs from %s - %s", uri, ex.displayText());
    }

    return Poco::JSON::Array::Ptr(); //empty JSON array
}

/***********************************************************************
 * Block Cache impl
 **********************************************************************/
class BlockCache : public QObject
{
    Q_OBJECT
public:

    BlockCache(QObject *parent):
        QObject(parent),
        _watcher(new QFutureWatcher<Poco::JSON::Array::Ptr>(this)),
        _registryPathToBlockDesc(getRegistryPathToBlockDesc())
    {
        connect(_watcher, SIGNAL(resultReadyAt(int)), this, SLOT(handleWatcherDone(int)));
        connect(_watcher, SIGNAL(finished()), this, SLOT(handleWatcherFinished()));
    }

signals:
    void blockDescUpdate(const Poco::JSON::Array::Ptr &);
    void blockDescReady(void);

public slots:
    void handleUpdate(void)
    {
        if (_watcher->isRunning()) return;

        //nodeKeys cannot be a temporary because QtConcurrent will reference them
        _allNodeKeys = Pothos::RemoteNode::listRegistryKeys();
        _watcher->setFuture(QtConcurrent::mapped(_allNodeKeys, &queryBlockDescs));
    }

private slots:
    void handleWatcherFinished(void)
    {
        //remove old nodes
        std::map<std::string, Poco::JSON::Array::Ptr> newMap;
        for (const auto &key : _allNodeKeys) newMap[key] = _nodeKeyToBlockDescs[key];
        _nodeKeyToBlockDescs = newMap;

        //map paths to block descs
        _registryPathToBlockDesc.clear();
        for (const auto &pair : _nodeKeyToBlockDescs)
        {
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

    void handleWatcherDone(const int which)
    {
        _nodeKeyToBlockDescs[_allNodeKeys[which]] = _watcher->resultAt(which);
    }

private:
    std::vector<std::string> _allNodeKeys;
    QFutureWatcher<Poco::JSON::Array::Ptr> *_watcher;

    //storage structures
    std::map<std::string, Poco::JSON::Array::Ptr> _nodeKeyToBlockDescs;
    std::map<std::string, Poco::JSON::Object::Ptr> &_registryPathToBlockDesc;
    Poco::JSON::Array::Ptr _superSetBlockDescs;
};

QObject *makeBlockCache(QObject *parent)
{
    return new BlockCache(parent);
}

#include "BlockCache.moc"
