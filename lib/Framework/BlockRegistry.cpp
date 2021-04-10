// Copyright (c) 2014-2017 Josh Blum
//                    2021 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/Block.hpp>
#include <Pothos/Framework/Topology.hpp>
#include <Pothos/Framework/BlockRegistry.hpp>
#include <Pothos/Framework/Exception.hpp>
#include <Pothos/Plugin.hpp>
#include <Poco/Logger.h>
#include <iostream>

//! Helper function to check the signature of an "opaque" call
static bool isOpaqueFactory(const Pothos::Callable &factory)
{
    return factory.getNumArgs() == 2 and
        factory.type(-1) == typeid(Pothos::Object) and
        factory.type(0) == typeid(const Pothos::Object *) and
        factory.type(1) == typeid(const size_t);
}

/***********************************************************************
 * Instance of BlockRegistry peforms check and plugin registration
 **********************************************************************/
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
        fullPath = PluginPath("/blocks", path);
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
        factory.type(-1) == typeid(std::shared_ptr<Topology>) or
        isOpaqueFactory(factory))
    {
        //register
        try
        {
            PluginRegistry::add(fullPath, factory);
        }
        catch (const PluginRegistryError &ex)
        {
            poco_error(Poco::Logger::get("Pothos.BlockRegistry"), ex.displayText());
            return;
        }
    }

    //otherwise report the error
    else
    {
        poco_error_f1(Poco::Logger::get("Pothos.BlockRegistry"), "Bad Factory, must return Block* or Topology*: %s", factory.toString());
    }
}

/***********************************************************************
 * BlockRegistry factory - retrieve factory and instantiate with args
 **********************************************************************/
static Pothos::Object blockRegistryMake(const std::string &path, const Pothos::Object *args, const size_t numArgs)
{
    const auto pluginPath = Pothos::PluginPath("/blocks", path);
    const auto plugin = Pothos::PluginRegistry::get(pluginPath);
    const auto factory = plugin.getObject().extract<Pothos::Callable>();

    //handle opaque factory case
    if (isOpaqueFactory(factory)) return factory.call<Pothos::Object>(args, numArgs);

    //check that the number of args match
    if (numArgs != factory.getNumArgs()) throw Pothos::InvalidArgumentException(
        "Pothos::BlockRegistry("+path+")", Poco::format(
        "factory expected %z args, but got %z args",
        factory.getNumArgs(), numArgs));

    //handle factories that return Block pointer types
    if (factory.type(-1) == typeid(Pothos::Block*))
    {
        Pothos::Block *element = factory.opaqueCall(args, numArgs);
        if (element->getName().empty()) element->setName(path); //a better name
        element->holdRef(Pothos::Object(plugin.getModule()));
        return Pothos::Object(std::shared_ptr<Pothos::Block>(element));
    }

    //handle factories that return Block shared pointer types
    if (factory.type(-1) == typeid(std::shared_ptr<Pothos::Block>))
    {
        std::shared_ptr<Pothos::Block> element = factory.opaqueCall(args, numArgs);
        if (element->getName().empty()) element->setName(path); //a better name
        element->holdRef(Pothos::Object(plugin.getModule()));
        return Pothos::Object(element);
    }

    //handle factories that return Topology pointer types
    if (factory.type(-1) == typeid(Pothos::Topology*))
    {
        Pothos::Topology *element = factory.opaqueCall(args, numArgs);
        if (element->getName().empty()) element->setName(path); //a better name
        element->holdRef(Pothos::Object(plugin.getModule()));
        return Pothos::Object(std::shared_ptr<Pothos::Topology>(element));
    }

    //handle factories that return Topology shared pointer types
    if (factory.type(-1) == typeid(std::shared_ptr<Pothos::Topology>))
    {
        std::shared_ptr<Pothos::Topology> element = factory.opaqueCall(args, numArgs);
        if (element->getName().empty()) element->setName(path); //a better name
        element->holdRef(Pothos::Object(plugin.getModule()));
        return Pothos::Object(element);
    }

    throw Pothos::IllegalStateException("Pothos::BlockRegistry::make("+path+")", factory.toString());
}

bool Pothos::BlockRegistry::doesBlockExist(const std::string &path)
{
    const auto blockPluginPath = "/blocks" + path;
    return Pothos::PluginRegistry::exists(blockPluginPath);
}

#include <Pothos/Managed.hpp>

static auto managedBlockRegistry = Pothos::ManagedClass()
    .registerClass<Pothos::BlockRegistry>()
    .registerWildcardStaticMethod(&blockRegistryMake)
    .registerStaticMethod(POTHOS_FCN_TUPLE(Pothos::BlockRegistry, doesBlockExist))
    .commit("Pothos/BlockRegistry");
