// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "ThreadPoolEval.hpp"
#include "EnvironmentEval.hpp"
#include <Pothos/Proxy.hpp>
#include <Pothos/Framework/ThreadPool.hpp>
#include <Poco/Logger.h>
#include <sstream>

ThreadPoolEval::ThreadPoolEval(void):
    _failureState(false)
{
    return;
}

ThreadPoolEval::~ThreadPoolEval(void)
{
    return;
}

void ThreadPoolEval::acceptConfig(const Poco::JSON::Object::Ptr &config)
{
    _newZoneConfig = config;
}

void ThreadPoolEval::acceptEnvironment(const std::shared_ptr<EnvironmentEval> &env)
{
    _newEnvironmentEval = env;
}

Pothos::Proxy ThreadPoolEval::makeThreadPool(void)
{
    if (not _newZoneConfig) return Pothos::Proxy();

    auto env = _newEnvironmentEval->getEnv();
    const auto &config = _newZoneConfig;

    //load the args
    std::stringstream ss; config->stringify(ss);
    Pothos::ThreadPoolArgs args(ss.str());

    //create the thread pool
    return env->findProxy("Pothos/ThreadPool").callProxy("new", args);
}

void ThreadPoolEval::update(void)
{
    _newEnvironment = _newEnvironmentEval->getEnv();
    if (_newEnvironmentEval->isFailureState())
    {
        _errorMsg = _newEnvironmentEval->getErrorMsg();
        _failureState = true;
        return;
    }

    //evaluation environment change?
    bool requireNewThreadPool = _newEnvironment != _lastEnvironment;

    //zone configuration change?
    if (_newZoneConfig and _lastZoneConfig)
    {
        //a config change of any kind means a new thread pool
        std::stringstream oldConfig, newConfig;
        _lastZoneConfig->stringify(oldConfig);
        _newZoneConfig->stringify(newConfig);
        if (oldConfig.str() != newConfig.str())
        {
            requireNewThreadPool = true;
        }
    }

    //make a new thread pool
    if (requireNewThreadPool)
    {
        try
        {
            _threadPool = this->makeThreadPool();
            _lastEnvironmentEval = _newEnvironmentEval;
            _lastEnvironment = _newEnvironment;
            _lastZoneConfig = _newZoneConfig;
            _failureState = false;
        }
        catch (const Pothos::Exception &ex)
        {
            poco_error(Poco::Logger::get("PothosGui.ThreadPoolEval.update"), ex.displayText());
            _errorMsg = QString::fromStdString(ex.displayText());
            _failureState = true;
        }
    }
}
