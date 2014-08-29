// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/Block.hpp>
#include <Pothos/Framework/Topology.hpp>
#include <Pothos/Framework/BlockRegistry.hpp>
#include <Pothos/Framework/Exception.hpp>
#include <Pothos/Plugin.hpp>
#include <Poco/Logger.h>

Pothos::BlockRegistry::BlockRegistry(const std::string &path, const Callable &factory)
{
    //check the path
    if (path.empty() or path.front() != '/')
    {
        poco_error_f1(Poco::Logger::get("Pothos.BlockRegistry"), "Invalid path: %s", path);
        return;
    }

    //parse the path
    PluginPath fullPath;
    try
    {
        fullPath = PluginPath("/blocks").join(path.substr(1));
    }
    catch (const PluginPathError &)
    {
        poco_error_f1(Poco::Logger::get("Pothos.BlockRegistry"), "Invalid path: %s", path);
        return;
    }

    //check the factory
    if (
        factory.type(-1) == typeid(Block*) or
        factory.type(-1) == typeid(std::shared_ptr<Block>) or
        factory.type(-1) == typeid(Topology*) or
        factory.type(-1) == typeid(std::shared_ptr<Topology>))
    {
        //register
        PluginRegistry::add(fullPath, factory);
    }

    //otherwise report the error
    else
    {
        poco_error_f1(Poco::Logger::get("Pothos.BlockRegistry"), "Bad Factory, must return Block* or Topology*: %s", factory.toString());
    }
}

static Pothos::Object blockRegistryMake(const std::string &path, const Pothos::Object *args, const size_t numArgs)
{
    const auto pluginPath = Pothos::PluginPath("/blocks").join(path.substr(1));
    const auto plugin = Pothos::PluginRegistry::get(pluginPath);
    const auto factory = plugin.getObject().extract<Pothos::Callable>();

    if (factory.type(-1) == typeid(Pothos::Block*))
    {
        auto element = factory.opaqueCall(args, numArgs).extract<Pothos::Block *>();
        if (element->getName().empty()) element->setName(path); //a better name
        element->holdRef(Pothos::Object(plugin.getModule()));
        return Pothos::Object(std::shared_ptr<Pothos::Block>(element));
    }

    if (factory.type(-1) == typeid(std::shared_ptr<Pothos::Block>))
    {
        auto element = factory.opaqueCall(args, numArgs).extract<std::shared_ptr<Pothos::Block>>();
        if (element->getName().empty()) element->setName(path); //a better name
        element->holdRef(Pothos::Object(plugin.getModule()));
        return Pothos::Object(element);
    }

    if (factory.type(-1) == typeid(Pothos::Topology*))
    {
        auto element = factory.opaqueCall(args, numArgs).extract<Pothos::Topology *>();
        if (element->getName().empty()) element->setName(path); //a better name
        element->holdRef(Pothos::Object(plugin.getModule()));
        return Pothos::Object(std::shared_ptr<Pothos::Topology>(element));
    }

    if (factory.type(-1) == typeid(std::shared_ptr<Pothos::Topology>))
    {
        auto element = factory.opaqueCall(args, numArgs).extract<std::shared_ptr<Pothos::Topology>>();
        if (element->getName().empty()) element->setName(path); //a better name
        element->holdRef(Pothos::Object(plugin.getModule()));
        return Pothos::Object(element);
    }

    throw Pothos::IllegalStateException("Pothos::BlockRegistry::make("+path+")", factory.toString());
}

#include <Pothos/Managed.hpp>

static auto managedBlockRegistry = Pothos::ManagedClass()
    .registerClass<Pothos::BlockRegistry>()
    .registerWildcardStaticMethod(&blockRegistryMake)
    .commit("Pothos/BlockRegistry");
