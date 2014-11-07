// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGuiUtils.hpp" //getObjectMap
#include "EvalEngine.hpp"
#include "GraphObjects/GraphBlock.hpp"
#include "HostExplorer/HostExplorerDock.hpp"
#include "AffinitySupport/AffinityZonesDock.hpp"

EvalEngine::EvalEngine(QObject *parent):
    QObject(parent),
    _evalThreadRunning(false)
{
    connect(getObjectMap()["hostExplorerDock"], SIGNAL(hostUriListChanged(void)), this, SLOT(handleHostUriListChanged(void)));
    connect(getObjectMap()["affinityZonesDock"], SIGNAL(zonesChanged(void)), this, SLOT(handleAffinityZonesChanged(void)));
    this->startEvalThread();
}

EvalEngine::~EvalEngine(void)
{
    this->stopEvalThread();
}

void EvalEngine::submitTopology(const GraphObjectList &graphObjects)
{
    //extract all relevant information to process this in another thread
    TopologyInfo evalInfo;

    //a list of connection information
    evalInfo.connectionInfo = EvalEngine::getConnectionInfo(graphObjects);

    //a list of block eval instructions
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
        evalInfo.blockInfo.push_back(blockInfo);
    }

    //send it to the processing thread
    {
        std::unique_lock<std::mutex> lock(_eventMutex);
        _evalThreadInfo = evalInfo;
    }
    _eventCond.notify_one();
}

void EvalEngine::handleHostUriListChanged(void)
{
    auto dock = dynamic_cast<HostExplorerDock *>(getObjectMap()["hostExplorerDock"]);
}

void EvalEngine::handleAffinityZonesChanged(void)
{
    auto dock = dynamic_cast<AffinityZonesDock *>(getObjectMap()["affinityZonesDock"]);
}

void EvalEngine::startEvalThread(void)
{
    _evalThreadRunning = true;
    _evalThread = std::thread(&EvalEngine::runEvalThread, this);
}

void EvalEngine::stopEvalThread(void)
{
    {
        std::unique_lock<std::mutex> lock(_eventMutex);
        _evalThreadRunning = false;
    }
    _eventCond.notify_one();
    _evalThread.join();
}

void EvalEngine::runEvalThread(void)
{
    TopologyInfo latestInfo;
    while (true)
    {
        //react to the latest info before locking again
        this->handleEvent(latestInfo);

        std::unique_lock<std::mutex> lock(_eventMutex);
        _eventCond.wait(lock);
        if (not _evalThreadRunning) return; //exit
        latestInfo = _evalThreadInfo;
    }
}

void EvalEngine::handleEvent(const TopologyInfo &info)
{
    //merge in the block info
    std::map<QString, std::shared_ptr<BlockEngine>> newBlockEngines;
    for (const auto &blockInfo : info.blockInfo)
    {
        
    }

    this->reEvalAll();
}

void EvalEngine::reEvalAll(void)
{
    
}
