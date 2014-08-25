// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "EvalBlockCache.hpp"
#include "GraphObjects/GraphBlock.hpp"
#include <iostream>

bool EvalBlockCache::hasCriticalChange(void) const
{
    if (_blockDesc->isArray("args")) for (auto arg : *_blockDesc->getArray("args"))
    {
        const auto propKey = arg.extract<std::string>();
        if (didPropKeyHaveChange(QString::fromStdString(propKey))) return true;
    }
    if (_blockDesc->isArray("calls")) for (auto call : *_blockDesc->getArray("calls"))
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

std::vector<Poco::JSON::Object::Ptr> EvalBlockCache::settersChangedList(void) const
{
    std::vector<Poco::JSON::Object::Ptr> changedList;
    if (_blockDesc->isArray("calls")) for (auto call : *_blockDesc->getArray("calls"))
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

bool EvalBlockCache::updateChangedProperties(GraphBlock *block)
{
    bool hasError = false;
    for (const auto &propKey : block->getProperties())
    {
        if (not this->didPropKeyHaveChange(propKey)) continue;
        const auto val = block->getPropertyValue(propKey).toStdString();
        try
        {
            auto obj = this->getProxyBlock().callProxy("evalProperty", propKey.toStdString(), val);
            this->cachePropTypeStr(propKey, obj.call<std::string>("getTypeString"));
            block->setPropertyErrorMsg(propKey, "");
        }
        catch (const Pothos::Exception &ex)
        {
            block->setPropertyErrorMsg(propKey, QString::fromStdString(ex.message()));
            hasError = true;
        }
    }
    return not hasError;
}

void EvalBlockCache::loadCacheIntoBlock(GraphBlock *block)
{
    for (const auto &pair : _propKeyToTypeStr)
    {
        block->setPropertyTypeStr(pair.first, pair.second);
    }

    block->setPortDesc(_inputPortDesc, _outputPortDesc);
}

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

void EvalBlockCache::cacheInputPortInfo(const std::vector<Pothos::PortInfo> &infos)
{
    _inputPortDesc = portInfosToJSON(infos);
}

void EvalBlockCache::cacheOutputPortInfo(const std::vector<Pothos::PortInfo> &infos)
{
    _outputPortDesc = portInfosToJSON(infos);
}

void EvalBlockCache::cachePropTypeStr(const QString &key, const std::string &type)
{
    _propKeyToTypeStr[key] = type;
}
