// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/BufferManager.hpp>
#include <Pothos/Framework/Exception.hpp>
#include <Pothos/Callable.hpp>
#include <Pothos/Plugin.hpp>

Pothos::BufferManagerArgs::BufferManagerArgs(void):
    numBuffers(4),
    bufferSize(8*1024),
    nodeAffinity(-1)
{
    return;
}

Pothos::BufferManager::Sptr Pothos::BufferManager::make(const std::string &name)
{
    Sptr manager;
    try
    {
        auto plugin = Pothos::PluginRegistry::get(Pothos::PluginPath("/framework/buffer_manager").join(name));
        auto callable = plugin.getObject().extract<Pothos::Callable>();
        manager = callable.call<Sptr>();
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

void Pothos::BufferManager::setCallback(const std::function<void(const ManagedBuffer &)> &callback)
{
    _callback = callback;
}
