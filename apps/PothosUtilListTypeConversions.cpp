// Copyright (c) 2021 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include "PothosUtil.hpp"

#include <Pothos/Callable.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/System/Paths.hpp>
#include <Pothos/Util/TypeInfo.hpp>

#include <Poco/Format.h>
#include <Poco/Path.h>

#include <iostream>
#include <string>
#include <vector>

static bool isConvertCallable(const Pothos::Callable& callable)
{
    if(callable.getNumArgs() != 1) return false;
    if(callable.type(-1) == typeid(void)) return false;

    return true;
}

// Assumption: callable type validated
static void getConvertTypeNames(
    const Pothos::Callable& callable,
    std::string* srcTypeOut,
    std::string* dstTypeOut)
{
    *srcTypeOut = Pothos::Util::typeInfoToString(callable.type(0));
    *dstTypeOut = Pothos::Util::typeInfoToString(callable.type(-1));
}

static void getTypeConversions(
    const Pothos::PluginPath& pluginPath,
    const std::string& typeName,
    std::vector<std::string>* convertibleFromOut,
    std::vector<std::string>* convertibleToOut)
{
    Pothos::Plugin plugin(pluginPath);
    if(!Pothos::PluginRegistry::empty(pluginPath))
    {
        plugin = Pothos::PluginRegistry::get(pluginPath);
    }
    const auto& pluginObj = plugin.getObject();

    if(pluginObj.type() == typeid(Pothos::Callable))
    {
        const auto& callable = pluginObj.extract<Pothos::Callable>();
        if(isConvertCallable(callable))
        {
            std::string srcType;
            std::string dstType;
            getConvertTypeNames(callable, &srcType, &dstType);

            if(srcType == typeName)      convertibleFromOut->emplace_back(std::move(dstType));
            else if(dstType == typeName) convertibleToOut->emplace_back(std::move(srcType));
        }
    }

    for(const auto& subpath: Pothos::PluginRegistry::list(pluginPath))
    {
        getTypeConversions(
            pluginPath.join(subpath),
            typeName,
            convertibleFromOut,
            convertibleToOut);
    }
}

void PothosUtilBase::printTypeConversions(const std::string&, const std::string&)
{
    Pothos::ScopedInit init;

    const auto typeName = this->config().getString("type");
    if (typeName.empty())
    {
        std::cout << ">>> Specify --type=typeName to list conversions..." << std::endl;
        return;
    }

    std::vector<std::string> convertibleFrom;
    std::vector<std::string> convertibleTo;
    getTypeConversions(
        Pothos::PluginPath("/object/convert"),
        typeName,
        &convertibleFrom,
        &convertibleTo);

    std::cout << "Convertible from " << typeName << ":" << std::endl;
    for(const auto& type: convertibleFrom)
    {
        std::cout << " * " << type << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Convertible to " << typeName << ":" << std::endl;
    for(const auto& type: convertibleTo)
    {
        std::cout << " * " << type << std::endl;
    }
}
