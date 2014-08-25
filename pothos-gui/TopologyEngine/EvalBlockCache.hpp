// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <QString>
#include <string>
#include <vector>
#include <map>

class GraphBlock;

class EvalBlockCache
{
public:
    EvalBlockCache(const Poco::JSON::Object::Ptr &blockDesc)
    {
        _blockDesc = blockDesc;
    }

    //! critical change? need to make a new block
    bool hasCriticalChange(void) const;

    //! any setters that changed so we can re-call them
    std::vector<Poco::JSON::Object::Ptr> settersChangedList(void) const;

    //! update changed properties - returns true on success
    bool updateChangedProperties(GraphBlock *block);

    /*******************************************************************
     * property tracking
     ******************************************************************/
    void loadCurrentValue(const QString &key, const QString &val)
    {
        _currentValues[key] = val;
    }

    void markUpdateDone(void)
    {
        _previousValues = _currentValues;
    }

    /*******************************************************************
     * caching eval result and re-inserting into block
     ******************************************************************/
    void loadCacheIntoBlock(GraphBlock *block);
    void cacheInputPortInfo(const std::vector<Pothos::PortInfo> &infos);
    void cacheOutputPortInfo(const std::vector<Pothos::PortInfo> &infos);
    void cachePropTypeStr(const QString &key, const std::string &type);

    /*******************************************************************
     * proxy containing the block
     ******************************************************************/
    Pothos::Proxy getProxyBlock(void) const
    {
        return _proxyBlock;
    }

    void setProxyBlock(const Pothos::Proxy &block)
    {
        _proxyBlock = block;
        _previousValues.clear();
    }

private:

    bool didPropKeyHaveChange(const QString &key) const
    {
        if (_currentValues.count(key) == 0) return true;
        if (_previousValues.count(key) == 0) return true;
        return _currentValues.at(key) != _previousValues.at(key);
    }

    Poco::JSON::Object::Ptr _blockDesc;
    std::map<QString, QString> _currentValues;
    std::map<QString, QString> _previousValues;

    Poco::JSON::Array::Ptr _inputPortDesc;
    Poco::JSON::Array::Ptr _outputPortDesc;
    std::map<QString, std::string> _propKeyToTypeStr;

    Pothos::Proxy _proxyBlock;
};
