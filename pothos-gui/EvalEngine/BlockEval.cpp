// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "BlockEval.hpp"
#include "GraphObjects/GraphBlock.hpp"
#include "ThreadPoolEval.hpp"
#include "EnvironmentEval.hpp"
#include <Pothos/Proxy.hpp>

BlockEval::BlockEval(void)
{
    return;
}

BlockEval::~BlockEval(void)
{
    return;
}

void BlockEval::acceptInfo(const BlockInfo &info)
{
    _newBlockInfo = info;
}

void BlockEval::acceptEnvironment(const std::shared_ptr<EnvironmentEval> &env)
{
    _newEnvironmentEval = env;
}

void BlockEval::acceptThreadPool(const std::shared_ptr<ThreadPoolEval> &tp)
{
    _newThreadPoolEval = tp;
}

/***********************************************************************
 * update performs block evaluation
 **********************************************************************/
void BlockEval::update(void)
{
    //the environment changed? clear everything
    if (_newEnvironmentEval != _lastEnvironmentEval)
    {
        _lastEnvironmentEval = _newEnvironmentEval;
        _lastBlockInfo = BlockInfo();
        _blockEval = Pothos::Proxy();
    }

    //special case: apply settings only
    //no critical changes, block already exists
    if (_blockEval and not this->hasCriticalChange())
    {
    }

    //otherwise, make a new block and all calls
    else
    {
        if (not _blockEval)
        {
            auto evalEnv = _lastEnvironmentEval->getEval();
            auto BlockEval = evalEnv.getEnvironment()->findProxy("Pothos/Util/BlockEval");
            _blockEval = BlockEval.callProxy("new", evalEnv);
        }
    }

    //scrape and post port info

    //post the most recent status into the block in the gui thread context
    QMetaObject::invokeMethod(this, "postStatusToBlock", Qt::QueuedConnection, Q_ARG(BlockStatus, _lastBlockStatus));
}

/***********************************************************************
 * update graph block with the latest status
 **********************************************************************/
void BlockEval::postStatusToBlock(const BlockStatus &status)
{
    auto &block = status.block;
    if (not block) return; //block no longer exists

    block->clearBlockErrorMsgs();
    for (const auto &pair : status.propertyTypeInfos)
    {
        block->setPropertyTypeStr(pair.first, pair.second);
    }
    for (const auto &pair : status.propertyErrorMsgs)
    {
        block->setPropertyErrorMsg(pair.first, pair.second);
    }
    for (const auto &errMsg : status.blockErrorMsgs)
    {
        block->addBlockErrorMsg(errMsg);
    }
    if (status.inPortDesc and status.outPortDesc)
    {
        block->setPortDesc(status.inPortDesc, status.outPortDesc);
    }
    block->update();
}

/***********************************************************************
 * helper routines to comprehend the block desc
 **********************************************************************/
bool BlockEval::hasCriticalChange(void) const
{
    const auto &blockDesc = _newBlockInfo.desc;

    if (blockDesc->isArray("args")) for (auto arg : *blockDesc->getArray("args"))
    {
        const auto propKey = arg.extract<std::string>();
        if (didPropKeyHaveChange(QString::fromStdString(propKey))) return true;
    }
    if (blockDesc->isArray("calls")) for (auto call : *blockDesc->getArray("calls"))
    {
        const auto callObj = call.extract<Poco::JSON::Object::Ptr>();
        if (callObj->getValue<std::string>("type") != "initializer") continue;
        for (auto arg : *callObj->getArray("args"))
        {
            const auto propKey = arg.extract<std::string>();
            if (didPropKeyHaveChange(QString::fromStdString(propKey))) return true;
        }
    }
    return false;
}

std::vector<Poco::JSON::Object::Ptr> BlockEval::settersChangedList(void) const
{
    const auto &blockDesc = _newBlockInfo.desc;

    std::vector<Poco::JSON::Object::Ptr> changedList;
    if (blockDesc->isArray("calls")) for (auto call : *blockDesc->getArray("calls"))
    {
        const auto callObj = call.extract<Poco::JSON::Object::Ptr>();
        if (callObj->getValue<std::string>("type") != "setter") continue;
        for (auto arg : *callObj->getArray("args"))
        {
            const auto propKey = arg.extract<std::string>();
            if (didPropKeyHaveChange(QString::fromStdString(propKey)))
            {
                changedList.push_back(callObj);
            }
        }
    }
    return changedList;
}

bool BlockEval::didPropKeyHaveChange(const QString &key) const
{
    if (_newBlockInfo.properties.count(key) == 0) return true;
    if (_lastBlockInfo.properties.count(key) == 0) return true;
    return _newBlockInfo.properties.at(key) != _lastBlockInfo.properties.at(key);
}
