// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/Block.hpp>
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
    if (factory.type(-1) != typeid(Block*))
    {
        poco_error_f1(Poco::Logger::get("Pothos.BlockRegistry"), "Bad Factory, must return Block*: %s", factory.toString());
        return;
    }

    //register
    PluginRegistry::add(fullPath, factory);
}

Pothos::Callable Pothos::BlockRegistry::lookup(const std::string &path)
{
    return PluginRegistry::get(PluginPath("/blocks").join(path.substr(1))).getObject().extract<Callable>();
}

static Pothos::Object blockRegistryMake(const std::string &path, const Pothos::Object *args, const size_t numArgs)
{
    const auto factory = Pothos::BlockRegistry::lookup(path);
    auto block = factory.opaqueCall(args, numArgs).extract<Pothos::Block *>();
    //make a more interesting block name
    if (block->getName().empty() or block->getName() == "Block") block->setName(path);
    return Pothos::Object(std::shared_ptr<Pothos::Block>(block));
}

#include <Pothos/Managed.hpp>

static auto managedBlockRegistry = Pothos::ManagedClass()
    .registerClass<Pothos::BlockRegistry>()
    .registerWildcardStaticMethod(&blockRegistryMake)
    .commit("Pothos/BlockRegistry");
