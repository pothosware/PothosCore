// Copyright (c) 2013-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/BufferManager.hpp>
#include <Pothos/Framework/Exception.hpp>
#include <Pothos/Callable.hpp>
#include <Pothos/Plugin.hpp>
#include <cassert>

Pothos::BufferManagerArgs::BufferManagerArgs(void):
    numBuffers(4),
    bufferSize(8*1024),
    nodeAffinity(-1)
{
    return;
}

Pothos::BufferManager::BufferManager(void):
    _initialized(false)
{
    return;
}

Pothos::BufferManager::~BufferManager(void)
{
    return;
}

Pothos::BufferManager::Sptr Pothos::BufferManager::make(const std::string &name)
{
    Sptr manager;
    try
    {
        const auto plugin = Pothos::PluginRegistry::get(Pothos::PluginPath("/framework/buffer_manager").join(name));
        const auto &callable = plugin.getObject().extract<Pothos::Callable>();
        manager = callable.call();
    }
    catch(const Exception &ex)
    {
        throw Pothos::BufferManagerFactoryError("Pothos::BufferManager::make()", ex);
    }
    return manager;
}

Pothos::BufferManager::Sptr Pothos::BufferManager::make(const std::string &name, const BufferManagerArgs &args)
{
    auto manager = make(name);
    manager->init(args);
    return manager;
}

void Pothos::BufferManager::init(const BufferManagerArgs &)
{
    if(_initialized) throw BufferManagerFactoryError("Pothos::BufferManager::init()", "already initialized");
    _initialized = true;
}

void Pothos::BufferManager::setCallback(const std::function<void(const ManagedBuffer &)> &callback)
{
    _callback = callback;
}
