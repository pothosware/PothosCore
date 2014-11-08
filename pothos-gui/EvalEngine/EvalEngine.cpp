// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGuiUtils.hpp" //getObjectMap
#include "EvalEngine.hpp"
#include "BlockEval.hpp"
#include "ThreadPoolEval.hpp"
#include "EnvironmentEval.hpp"
#include "TopologyTraversal.hpp"
#include "GraphObjects/GraphBlock.hpp"
#include "AffinitySupport/AffinityZonesDock.hpp"
#include <memory>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <map>
#include <thread>
#include <cassert>

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
    std::map<HostProcPair, std::shared_ptr<EnvironmentEval>> environmentEvals;
    std::map<QString, std::shared_ptr<ThreadPoolEval>> threadPoolEvals;
    std::map<GraphBlock *, std::shared_ptr<BlockEval>> blockEvals;
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
        blockInfo.isGraphWidget = block->isGraphWidget();
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

    std::map<GraphBlock *, std::shared_ptr<BlockEval>> newBlockEvals;
    std::map<QString, std::shared_ptr<ThreadPoolEval>> newThreadPoolEvals;
    std::map<HostProcPair, std::shared_ptr<EnvironmentEval>> newEnvironmentEvals;

    //merge in the block info
    for (const auto &blockInfo : latestInfo.blockInfo)
    {
        auto blockPtr = blockInfo.block.data();
        const auto &zone = blockInfo.zone;

        //extract the configuration for this zone
        Poco::JSON::Object::Ptr config;
        {
            auto it = latestInfo.zoneInfo.find(zone);
            if (it != latestInfo.zoneInfo.end()) config = it->second;
        }
        const auto hostProcKey = EnvironmentEval::getHostProcFromConfig(config);

        //copy the block eval or make a new one
        auto &blockEval = newBlockEvals[blockPtr];
        if (not blockEval)
        {
            auto it = _impl->blockEvals.find(blockPtr);
            if (it != _impl->blockEvals.end()) blockEval = it->second;
            else blockEval.reset(new BlockEval());
        }

        //copy the thread pool or make a new one
        auto &threadPoolEval = newThreadPoolEvals[zone];
        if (not threadPoolEval)
        {
            auto it = _impl->threadPoolEvals.find(zone);
            if (it != _impl->threadPoolEvals.end()) threadPoolEval = _impl->threadPoolEvals.at(zone);
            else threadPoolEval.reset(new ThreadPoolEval());
        }

        //copy the eval environment or make a new one
        auto &envEval = newEnvironmentEvals[hostProcKey];
        if (not envEval)
        {
            auto it = _impl->environmentEvals.find(hostProcKey);
            if (it != _impl->environmentEvals.end()) envEval = it->second;
            envEval.reset(new EnvironmentEval());
        }

        //pass config into the environment
        assert(envEval);
        envEval->acceptConfig(zone, config);

        //pass config and env into thread pool
        assert(threadPoolEval);
        threadPoolEval->acceptConfig(config);
        threadPoolEval->acceptEnvironment(envEval);

        //pass info, env, and thread pool into block
        assert(blockEval);
        blockEval->acceptInfo(blockInfo);
        blockEval->acceptThreadPool(threadPoolEval);
        blockEval->acceptEnvironment(envEval);
    }

    //swap in the latest engines that are in-use
    _impl->blockEvals = newBlockEvals;
    _impl->threadPoolEvals = newThreadPoolEvals;
    _impl->environmentEvals = newEnvironmentEvals;

    //1) update all environments in case there were changes
    for (auto &pair : _impl->environmentEvals) pair.second->update();
    //2) update all thread pools in case there were changes
    for (auto &pair : _impl->threadPoolEvals) pair.second->update();
    //3) update all the blocks in case there were changes
    for (auto &pair : _impl->blockEvals) pair.second->update();
}
