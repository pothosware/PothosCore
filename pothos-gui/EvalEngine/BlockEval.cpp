// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "BlockEval.hpp"
#include "GraphObjects/GraphBlock.hpp"
#include "ThreadPoolEval.hpp"
#include "EnvironmentEval.hpp"
#include <Pothos/Proxy.hpp>
#include <Pothos/Framework.hpp>
#include <Poco/Logger.h>
#include <QWidget>
#include <cassert>
#include <iostream>
#include <QApplication>

//! helper to convert the port info vector into JSON for serialization of the block
static Poco::JSON::Array::Ptr portInfosToJSON(const std::vector<Pothos::PortInfo> &infos)
{
    Poco::JSON::Array::Ptr array = new Poco::JSON::Array();
    for (const auto &info : infos)
    {
        Poco::JSON::Object::Ptr portInfo = new Poco::JSON::Object();
        portInfo->set("name", info.name);
        portInfo->set("isSigSlot", info.isSigSlot);
        portInfo->set("size", info.dtype.size());
        portInfo->set("dtype", info.dtype.toString());
        array->add(portInfo);
    }
    return array;
}

BlockEval::BlockEval(void)
{
    qRegisterMetaType<BlockStatus>("BlockStatus");
    this->moveToThread(QApplication::instance()->thread());
}

BlockEval::~BlockEval(void)
{
    return;
}

bool BlockEval::isInfoMatch(const BlockInfo &info) const
{
    if (info.id != _newBlockInfo.id) return false;
    if (not info.desc) return false;
    if (not _newBlockInfo.desc) return false;
    return info.desc->getValue<std::string>("path") == _newBlockInfo.desc->getValue<std::string>("path");
}

bool BlockEval::isReady(void) const
{
    if (not _newBlockInfo.enabled) return false;

    if (not _lastBlockStatus.blockErrorMsgs.empty()) return false;

    for (const auto &pair : _lastBlockStatus.propertyErrorMsgs)
    {
        if (not pair.second.isEmpty()) return false;
    }

    if (not _lastBlockStatus.inPortDesc) return false;

    if (not _lastBlockStatus.outPortDesc) return false;

    return true;
}

Pothos::Proxy BlockEval::getProxyBlock(void) const
{
    return _blockEval.callProxy("getProxyBlock");
}

void BlockEval::acceptInfo(const BlockInfo &info)
{
    _newBlockInfo = info;
    assert(_newBlockInfo.desc);
    _lastBlockStatus.block = _newBlockInfo.block;
}

void BlockEval::acceptEnvironment(const std::shared_ptr<EnvironmentEval> &env)
{
    _newEnvironmentEval = env;
}

void BlockEval::acceptThreadPool(const std::shared_ptr<ThreadPoolEval> &tp)
{
    _newThreadPoolEval = tp;
}

void BlockEval::update(void)
{
    _newEnvironment = _newEnvironmentEval->getEnv();
    _newThreadPool = _newThreadPoolEval->getThreadPool();

    //clear old error messages -- lets make new ones
    _lastBlockStatus.blockErrorMsgs.clear();
    _lastBlockStatus.propertyErrorMsgs.clear();

    //perform evaluation
    const bool evalSuccess = this->evaluationProcedure();

    //When eval fails, do a re-check on the environment.
    //Because block eval could have killed the environment.
    if (not evalSuccess) _newEnvironmentEval->update();

    //When environment fails, replace the block error messages
    //with the error message from the evaluation environment.
    if (_newEnvironmentEval->isFailureState())
    {
        _lastBlockStatus.blockErrorMsgs.clear();
        _lastBlockStatus.propertyErrorMsgs.clear();
        _lastBlockStatus.blockErrorMsgs.push_back(_newEnvironmentEval->getErrorMsg());
    }

    //we should have at least one error reported when not success
    assert(evalSuccess or not _lastBlockStatus.blockErrorMsgs.empty());

    //post the most recent status into the block in the gui thread context
    QMetaObject::invokeMethod(this, "postStatusToBlock", Qt::QueuedConnection, Q_ARG(BlockStatus, _lastBlockStatus));
}

/***********************************************************************
 * evaluation procedure implementation
 **********************************************************************/
bool BlockEval::evaluationProcedure(void)
{
    if (_newEnvironmentEval->isFailureState())
    {
        _lastBlockStatus.blockErrorMsgs.push_back(_newEnvironmentEval->getErrorMsg());
        return false;
    }
    bool evalSuccess = true;

    //the environment changed? clear everything
    if (_newEnvironment != _lastEnvironment)
    {
        std::cout << "env changed " << _newBlockInfo.id.toStdString() <<  "!!\n";
        _lastEnvironmentEval = _newEnvironmentEval;
        _lastEnvironment = _newEnvironment;
        _lastBlockInfo = BlockInfo();
        _blockEval = Pothos::Proxy();
    }

    //special case: apply settings only
    //no critical changes, block already exists
    if (_blockEval and not this->hasCriticalChange())
    {
        //update all properties - regardless of changes
        bool setterError = not this->updateAllProperties();
        if (not setterError) for (const auto &setter : this->settersChangedList())
        {
            try
            {
                _blockEval.callVoid("handleCall", setter);
            }
            catch (const Pothos::Exception &ex)
            {
                this->reportError(setter->getValue<std::string>("name"), ex);
                setterError = true;
                break;
            }
        }
        if (setterError) evalSuccess = false;
    }

    //otherwise, make a new block and all calls
    //update all properties - regardless of changes
    //this may create a new _blockEval if needed
    else if (this->updateAllProperties())
    {
        try
        {
            //widget blocks have to be evaluated in the GUI thread context, otherwise, eval here
            if (_newBlockInfo.isGraphWidget)
            {
                QMetaObject::invokeMethod(this, "blockEvalInGUIContext", Qt::BlockingQueuedConnection);
                if (not _blockEval) evalSuccess = false;
                else _lastBlockStatus.widget = this->getProxyBlock().call<QWidget *>("widget");
            }
            else _blockEval.callProxy("eval", _newBlockInfo.id.toStdString(), _newBlockInfo.desc);
        }
        catch(const Pothos::Exception &ex)
        {
            this->reportError("eval", ex);
            evalSuccess = false;
        }
    }
    else evalSuccess = false;

    //validate the id
    if (_newBlockInfo.id.isEmpty())
    {
        _lastBlockStatus.blockErrorMsgs.push_back(tr("Error: empty ID"));
    }

    //load its port info
    if (evalSuccess) try
    {
        auto proxyBlock = this->getProxyBlock();
        _lastBlockStatus.inPortDesc = portInfosToJSON(proxyBlock.call<std::vector<Pothos::PortInfo>>("inputPortInfo"));
        _lastBlockStatus.outPortDesc = portInfosToJSON(proxyBlock.call<std::vector<Pothos::PortInfo>>("outputPortInfo"));
    }

    //parser errors report
    catch(const Pothos::Exception &ex)
    {
        this->reportError("portInfo", ex);
        evalSuccess = false;
    }

    //set the thread pool
    if (evalSuccess and _newThreadPoolEval->isFailureState())
    {
        _lastBlockStatus.blockErrorMsgs.push_back(_newThreadPoolEval->getErrorMsg());
        evalSuccess = false;
    }

    //set the thread pool
    if (evalSuccess and not (_newThreadPool == _lastThreadPool))
    {
        try
        {
            if (_newThreadPool) this->getProxyBlock().callVoid("setThreadPool", _newThreadPool);
            _lastThreadPool = _newThreadPool;
            _lastThreadPoolEval = _newThreadPoolEval;
        }
        catch(const Pothos::Exception &ex)
        {
            this->reportError("setThreadPool", ex);
            evalSuccess = false;
        }
    }

    //report block level error when properties errors are present
    if (not evalSuccess and _lastBlockStatus.blockErrorMsgs.empty())
    {
        for (const auto &pair : _lastBlockStatus.propertyErrorMsgs)
        {
            if (pair.second.isEmpty()) continue;
            _lastBlockStatus.blockErrorMsgs.push_back(tr("Error: cannot evaluate this block with property errors"));
            break;
        }
    }

    //stash the most recent state
    if (evalSuccess) _lastBlockInfo = _newBlockInfo;

    return evalSuccess;
}

/***********************************************************************
 * update graph block with the latest status
 **********************************************************************/
void BlockEval::postStatusToBlock(const BlockStatus &status)
{
    auto &block = status.block;
    if (not block) return; //block no longer exists

    //clear old error messages
    block->clearBlockErrorMsgs();
    for (const auto &propKey : block->getProperties())
    {
        block->setPropertyErrorMsg(propKey, "");
    }

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
    block->setGraphWidget(status.widget);

    block->update(); //cause redraw after changes
    emit block->evalDoneEvent(); //trigger done event subscribers
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

bool BlockEval::updateAllProperties(void)
{
    if (not _blockEval) try
    {
        auto evalEnv = _newEnvironmentEval->getEval();
        auto BlockEval = evalEnv.getEnvironment()->findProxy("Pothos/Util/BlockEval");
        _blockEval = BlockEval.callProxy("new", evalEnv);
        _lastThreadPoolEval.reset();
    }
    catch (const Pothos::Exception &ex)
    {
        this->reportError("make", ex);
        return false;
    }

    bool hasError = false;
    for (const auto &pair : _newBlockInfo.properties)
    {
        const auto &propKey = pair.first;
        const auto &propVal = pair.second;
        try
        {
            auto obj = _blockEval.callProxy("evalProperty", propKey.toStdString(), propVal.toStdString());
            _lastBlockStatus.propertyTypeInfos[propKey] = obj.call<std::string>("getTypeString");
        }
        catch (const Pothos::Exception &ex)
        {
            _lastBlockStatus.propertyErrorMsgs[propKey] = QString::fromStdString(ex.message());
            hasError = true;
        }
    }
    return not hasError;
}

void BlockEval::reportError(const std::string &action, const Pothos::Exception &ex)
{
    poco_error_f2(Poco::Logger::get("PothosGui.BlockEval."+action),
        "%s(...) - %s", _newBlockInfo.id.toStdString(), ex.displayText());
    _lastBlockStatus.blockErrorMsgs.push_back(tr("%1::%2(...) - %3")
        .arg(_newBlockInfo.id)
        .arg(QString::fromStdString(action))
        .arg(QString::fromStdString(ex.displayText())));
}

void BlockEval::blockEvalInGUIContext(void)
{
    try
    {
        _blockEval.callProxy("eval", _newBlockInfo.id.toStdString(), _newBlockInfo.desc);
    }
    catch (const Pothos::Exception &ex)
    {
        _blockEval = Pothos::Proxy();
        poco_error_f2(Poco::Logger::get("PothosGui.BlockEval.guiEval"), "%s-%s", _newBlockInfo.id.toStdString(), ex.displayText());
        _lastBlockStatus.blockErrorMsgs.push_back(tr("Failed to eval in GUI context %1-%2").arg(_newBlockInfo.id).arg(QString::fromStdString(ex.message())));
    }
}
