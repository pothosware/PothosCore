// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "ThreadPoolEval.hpp"
#include <sstream>

ThreadPoolEval::ThreadPoolEval(void)
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
    
}

void ThreadPoolEval::update(void)
{
    //a config change of any kind means a new thread pool
    std::stringstream oldConfig, newConfig;
    _lastZoneConfig->stringify(oldConfig);
    _newZoneConfig->stringify(newConfig);
    if (oldConfig.str() != newConfig.str())
    {
        //TODO make new thread pool
    }
}
