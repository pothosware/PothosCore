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
    auto env = _newEnvironmentEval->getEnv();
    const auto &config = _newZoneConfig;

    //load the args
    Pothos::ThreadPoolArgs args;
    if (config->has("numThreads"))
    {
        args.numThreads = config->getValue<int>("numThreads");
    }
    if (config->has("priority"))
    {
        args.priority = config->getValue<double>("priority");
    }
    if (config->has("affinityMode") and config->has("affinityMask"))
    {
        args.affinityMode = config->getValue<std::string>("affinityMode");
        auto mask = config->getArray("affinityMask");
        for (size_t i = 0; i < mask->size(); i++) args.affinity.push_back(mask->getElement<int>(i));
    }
    if (config->has("yieldMode"))
    {
        args.yieldMode = config->getValue<std::string>("yieldMode");
    }

    //create the thread pool
    return env->findProxy("Pothos/ThreadPool").callProxy("new", args);
}

void ThreadPoolEval::update(void)
{
    if (_newEnvironmentEval->isFailureState())
    {
        _failureState = true;
    }
    if (this->isFailureState()) return;

    bool requireNewThreadPool = false;

    //evaluation environment change?
    if (_lastEnvironmentEval != _newEnvironmentEval)
    {
        _lastEnvironmentEval = _newEnvironmentEval;
        requireNewThreadPool = true;
    }

    //zone configuration change?
    if (not _newZoneConfig)
    {
        _threadPool = Pothos::Proxy();
    }
    else if (_newZoneConfig and _lastZoneConfig)
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
    _lastZoneConfig = _newZoneConfig;

    //make a new thread pool
    if (requireNewThreadPool and _newEnvironmentEval and _newZoneConfig)
    {
        try
        {
            _threadPool = this->makeThreadPool();
        }
        catch (const Pothos::Exception &ex)
        {
            poco_error(Poco::Logger::get("PothosGui.ThreadPoolEval.update"), ex.displayText());
            _failureState = true;
        }
    }
}
