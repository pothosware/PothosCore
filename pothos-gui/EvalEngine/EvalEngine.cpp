// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGuiUtils.hpp" //getObjectMap
#include "EvalEngine.hpp"
#include "EvalEngineImpl.hpp"
#include "GraphObjects/GraphBlock.hpp"
#include "AffinitySupport/AffinityZonesDock.hpp"
#include <QSignalMapper>
#include <QThread>
#include <cassert>

EvalEngine::EvalEngine(QObject *parent):
    QObject(parent),
    _thread(new QThread()),
    _impl(new EvalEngineImpl()),
    _blockEvalMapper(new QSignalMapper(this)),
    _affinityDock(dynamic_cast<AffinityZonesDock *>(getObjectMap()["affinityZonesDock"]))
{
    assert(_affinityDock != nullptr);
    connect(_affinityDock, SIGNAL(zonesChanged(void)), this, SLOT(handleAffinityZonesChanged(void)));
    connect(_blockEvalMapper, SIGNAL(mapped(QObject *)), this, SLOT(submitBlock(QObject *)));

    _impl->moveToThread(_thread);
    _thread->start();
}

EvalEngine::~EvalEngine(void)
{
    delete _impl;
    _thread->quit();
    _thread->wait();
}

static BlockInfo blockToBlockInfo(GraphBlock *block)
{
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
    return blockInfo;
}

void EvalEngine::submitTopology(const GraphObjectList &graphObjects)
{
    //create list of block eval information
    BlockInfos blockInfos;
    for (auto obj : graphObjects)
    {
        auto block = dynamic_cast<GraphBlock *>(obj);
        if (block == nullptr) continue;
        _blockEvalMapper->setMapping(block, block);
        connect(block, SIGNAL(triggerEvalEvent(void)), _blockEvalMapper, SLOT(map(void)));
        blockInfos[block] = blockToBlockInfo(block);
    }

    //create a list of connection eval information
    const auto connInfos = TopologyEval::getConnectionInfo(graphObjects);

    //submit the information to the eval thread object
    QMetaObject::invokeMethod(_impl, "submitTopology", Qt::QueuedConnection, Q_ARG(BlockInfos, blockInfos), Q_ARG(ConnectionInfos, connInfos));
}

void EvalEngine::submitActivateTopology(const bool active)
{
    QMetaObject::invokeMethod(_impl, "submitActivateTopology", Qt::QueuedConnection, Q_ARG(bool, active));
}

void EvalEngine::submitBlock(QObject *obj)
{
    auto block = dynamic_cast<GraphBlock *>(obj);
    assert(block != nullptr);
    QMetaObject::invokeMethod(_impl, "submitBlock", Qt::QueuedConnection, Q_ARG(BlockInfo, blockToBlockInfo(block)));
}

std::string EvalEngine::getTopologyDotMarkup(const bool arg)
{
    std::string result;
    QMetaObject::invokeMethod(_impl, "getTopologyDotMarkup", Qt::BlockingQueuedConnection, Q_RETURN_ARG(std::string, result), Q_ARG(bool, arg));
    return result;
}

void EvalEngine::handleAffinityZonesChanged(void)
{
    ZoneInfos zoneInfos;
    for (const auto &zoneName : _affinityDock->zones())
    {
        zoneInfos[zoneName] = _affinityDock->zoneToConfig(zoneName);
    }
    QMetaObject::invokeMethod(_impl, "submitZoneInfo", Qt::QueuedConnection, Q_ARG(ZoneInfos, zoneInfos));
}
