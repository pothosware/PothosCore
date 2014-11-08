// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "EvalEngineImpl.hpp"
#include "EvalEngine.hpp"
#include "BlockEval.hpp"
#include "ThreadPoolEval.hpp"
#include "EnvironmentEval.hpp"
#include "TopologyEval.hpp"
#include "GraphObjects/GraphBlock.hpp"
#include <QThread>
#include <QAbstractEventDispatcher>
#include <cassert>

EvalEngineImpl::EvalEngineImpl(void)
{
    qRegisterMetaType<BlockInfo>("BlockInfo");
    qRegisterMetaType<BlockInfos>("BlockInfos");
    qRegisterMetaType<ConnectionInfo>("ConnectionInfo");
    qRegisterMetaType<ConnectionInfos>("ConnectionInfos");
    qRegisterMetaType<ZoneInfos>("ZoneInfos");
}

EvalEngineImpl::~EvalEngineImpl(void)
{
    return;
}

void EvalEngineImpl::submitActivateTopology(const bool enable)
{
    //make a new topology evaluator only if enabled and DNE
    if (enable and _topologyEval) _topologyEval.reset(new TopologyEval());

    //if disabled, clear the current evaluator if present
    if (not enable) _topologyEval.reset();
}

void EvalEngineImpl::submitBlock(const BlockInfo &info)
{
    _blockInfo[info.block.data()] = info;
    this->reEvalAll();
}

void EvalEngineImpl::submitBlocks(const BlockInfos &info)
{
    _blockInfo = info;
    this->reEvalAll();
}

void EvalEngineImpl::submitConnections(const ConnectionInfos &info)
{
    _connectionInfo = info;
    this->reEvalAll();
}

void EvalEngineImpl::submitZoneInfo(const ZoneInfos &info)
{
    _zoneInfo = info;
    this->reEvalAll();
}

void EvalEngineImpl::reEvalAll(void)
{
    //Do not evaluate when there are pending events in the queue.
    //Evaluate only after all events received - AKA event compression.
    if (this->thread()->eventDispatcher()->hasPendingEvents()) return;

    std::map<GraphBlock *, std::shared_ptr<BlockEval>> newBlockEvals;
    std::map<QString, std::shared_ptr<ThreadPoolEval>> newThreadPoolEvals;
    std::map<HostProcPair, std::shared_ptr<EnvironmentEval>> newEnvironmentEvals;

    //merge in the block info
    for (const auto &blockInfoPair : _blockInfo)
    {
        auto blockPtr = blockInfoPair.first;
        const auto &blockInfo = blockInfoPair.second;
        const auto &zone = blockInfo.zone;

        //extract the configuration for this zone
        Poco::JSON::Object::Ptr config;
        {
            auto it = _zoneInfo.find(zone);
            if (it != _zoneInfo.end()) config = it->second;
        }
        const auto hostProcKey = EnvironmentEval::getHostProcFromConfig(config);

        //copy the block eval or make a new one
        auto &blockEval = newBlockEvals[blockPtr];
        if (not blockEval)
        {
            auto it = _blockEvals.find(blockPtr);
            if (it != _blockEvals.end()) blockEval = it->second;
            else blockEval.reset(new BlockEval());
        }

        //copy the thread pool or make a new one
        auto &threadPoolEval = newThreadPoolEvals[zone];
        if (not threadPoolEval)
        {
            auto it = _threadPoolEvals.find(zone);
            if (it != _threadPoolEvals.end()) threadPoolEval = _threadPoolEvals.at(zone);
            else threadPoolEval.reset(new ThreadPoolEval());
        }

        //copy the eval environment or make a new one
        auto &envEval = newEnvironmentEvals[hostProcKey];
        if (not envEval)
        {
            auto it = _environmentEvals.find(hostProcKey);
            if (it != _environmentEvals.end()) envEval = it->second;
            else envEval.reset(new EnvironmentEval());
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
    _blockEvals = newBlockEvals;
    _threadPoolEvals = newThreadPoolEvals;
    _environmentEvals = newEnvironmentEvals;

    //1) update all environments in case there were changes
    for (auto &pair : _environmentEvals) pair.second->update();
    //2) update all thread pools in case there were changes
    for (auto &pair : _threadPoolEvals) pair.second->update();
    //3) update all the blocks in case there were changes
    for (auto &pair : _blockEvals) pair.second->update();
    //4) update topology when present (activation mode)
    if (_topologyEval)
    {
        _topologyEval->acceptConnectionInfo(_connectionInfo);
        _topologyEval->acceptBlockEvals(_blockEvals);
        _topologyEval->update();
    }
}
