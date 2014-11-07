// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGuiUtils.hpp" //getObjectMap
#include "EvalEngine.hpp"
#include "ZoneEngine.hpp"
#include "BlockEngine.hpp"
#include "TopologyTraversal.hpp"
#include "GraphObjects/GraphBlock.hpp"
#include "AffinitySupport/AffinityZonesDock.hpp"
#include <memory>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <map>
#include <thread>

/*!
 * Information passed into the evaluation thread.
 */
struct EvalInfo
{
    EvalInfo(void):
        runEvalThread(false),
        topologyActive(false)
    {
        return;
    }
    bool runEvalThread;
    bool topologyActive;
    std::vector<BlockInfo> blockInfo;
    std::vector<ConnectionInfo> connectionInfo;
    std::map<QString, Poco::JSON::Object::Ptr> zoneInfo;
};

/***********************************************************************
 * Private implementation to deal with threading
 **********************************************************************/
struct EvalEngine::Impl
{
    //thread communication event queue
    std::mutex eventMutex;
    std::condition_variable eventCond;
    EvalInfo mainEvalInfo;
    std::queue<EvalInfo> queueEvalInfo;
    std::thread evalThread;

    //active evaluation hooks
    EvalInfo latestEvalInfo;
    std::map<QString, std::shared_ptr<ZoneEngine>> zoneEngines;
    std::map<GraphBlock *, std::shared_ptr<BlockEngine>> blockEngines;
};

/***********************************************************************
 * Eval engine implementation
 **********************************************************************/
EvalEngine::EvalEngine(QObject *parent):
    QObject(parent),
    _impl(new Impl())
{
    connect(getObjectMap()["affinityZonesDock"], SIGNAL(zonesChanged(void)), this, SLOT(handleAffinityZonesChanged(void)));
    this->startEvalThread();
}

EvalEngine::~EvalEngine(void)
{
    this->stopEvalThread();
    delete _impl;
}

void EvalEngine::submitTopology(const GraphObjectList &graphObjects)
{
    //extract all relevant information to process this in another thread:

    //a list of connection information
    _impl->mainEvalInfo.connectionInfo = TopologyTraversal::getConnectionInfo(graphObjects);

    //a list of block eval instructions
    _impl->mainEvalInfo.blockInfo.clear();
    for (auto obj : graphObjects)
    {
        auto block = dynamic_cast<GraphBlock *>(obj);
        if (block == nullptr) continue;
        BlockInfo blockInfo;
        blockInfo.block = block;
        blockInfo.id = block->getId();
        blockInfo.zone = block->getAffinityZone();
        blockInfo.desc = block->getBlockDesc();
        for (const auto &propKey : block->getProperties())
        {
            blockInfo.properties[propKey] = block->getPropertyValue(propKey);
            blockInfo.paramDescs[propKey] = block->getParamDesc(propKey);
        }
        _impl->mainEvalInfo.blockInfo.push_back(blockInfo);
    }

    this->submitInfo();
}

void EvalEngine::handleAffinityZonesChanged(void)
{
    auto dock = dynamic_cast<AffinityZonesDock *>(getObjectMap()["affinityZonesDock"]);
    _impl->mainEvalInfo.zoneInfo.clear();
    for (const auto &zoneName : dock->zones())
    {
        _impl->mainEvalInfo.zoneInfo[zoneName] = dock->zoneToConfig(zoneName);
    }
    this->submitInfo();
}

void EvalEngine::startEvalThread(void)
{
    _impl->mainEvalInfo.runEvalThread = true;
    _impl->evalThread = std::thread(&EvalEngine::runEvalThread, this);
}

void EvalEngine::stopEvalThread(void)
{
    _impl->mainEvalInfo.runEvalThread = false;
    this->submitInfo();
}

void EvalEngine::submitInfo(void)
{
    {
        std::unique_lock<std::mutex> lock(_impl->eventMutex);
        _impl->queueEvalInfo.push(_impl->mainEvalInfo);
    }
    _impl->eventCond.notify_one();
}

void EvalEngine::runEvalThread(void)
{
    bool _changed = false;
    while (true)
    {
        //check if there were changes and react
        //this has to happen in an unlocked state
        if (_changed)
        {
            _changed = false;
            this->reEvalAll();
        }

        //wait on the condition variable
        std::unique_lock<std::mutex> lock(_impl->eventMutex);
        _impl->eventCond.wait(lock);

        //keep the latest eval info
        while (not _impl->queueEvalInfo.empty())
        {
            if (_impl->queueEvalInfo.size() == 1)
            {
                _impl->latestEvalInfo = _impl->queueEvalInfo.front();
                _changed = true;
            }
            _impl->queueEvalInfo.pop();
        }
        if (not _impl->latestEvalInfo.runEvalThread) return; //exit
    }
}

/***********************************************************************
 * Evaluation implementation
 **********************************************************************/
void EvalEngine::reEvalAll(void)
{
    const auto &latestInfo = _impl->latestEvalInfo;

    std::map<GraphBlock *, std::shared_ptr<BlockEngine>> newBlockEngines;
    std::map<QString, std::shared_ptr<ZoneEngine>> newZoneEngines;

    //merge in the block info
    for (const auto &blockInfo : latestInfo.blockInfo)
    {
        auto blockPtr = blockInfo.block.data();
        const auto &zone = blockInfo.zone;

        //copy the block engine or make a new one
        {
            auto it = _impl->blockEngines.find(blockPtr);
            if (it != _impl->blockEngines.end()) newBlockEngines[blockPtr] = it->second;
            else newBlockEngines[blockPtr].reset(new BlockEngine());
            newBlockEngines[blockPtr]->acceptInfo(blockInfo);
        }

        //copy the zone or make a new one
        if (newZoneEngines.count(zone) != 0) continue;
        auto &zoneEngine = newZoneEngines[zone];
        {
            auto it = _impl->zoneEngines.find(zone);
            if (it != _impl->zoneEngines.end()) zoneEngine = _impl->zoneEngines.at(zone);
            else zoneEngine.reset(new ZoneEngine());
        }

        //set the latest config for this zone
        {
            Poco::JSON::Object::Ptr config;
            auto it = latestInfo.zoneInfo.find(zone);
            if (it != latestInfo.zoneInfo.end()) config = it->second;
            zoneEngine->acceptConfig(config);
        }
    }

    //swap in the latest engines that are in-use
    _impl->blockEngines = newBlockEngines;
    _impl->zoneEngines = newZoneEngines;

    //update all zones in case there were changes
    for (auto &pair : _impl->zoneEngines) pair.second->update();

    //update all the blocks in case there were changes
    for (auto &pair : _impl->blockEngines) pair.second->update();
}
