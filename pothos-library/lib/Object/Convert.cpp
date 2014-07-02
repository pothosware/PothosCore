// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Object/ObjectImpl.hpp>
#include <Pothos/Object/Exception.hpp>
#include <Pothos/Util/TypeInfo.hpp>
#include <Pothos/Callable.hpp>
#include <Pothos/Plugin.hpp>
#include <Poco/SingletonHolder.h>
#include <Poco/RWLock.h>
#include <Poco/Logger.h>
#include <Poco/Format.h>
#include <Poco/Hash.h>
#include <map>

/***********************************************************************
 * Global map structure for conversions
 **********************************************************************/
static Poco::RWLock &getMapMutex(void)
{
    static Poco::SingletonHolder<Poco::RWLock> sh;
    return *sh.get();
}

//singleton global map for all supported conversions
typedef std::map<size_t, Pothos::Plugin> ConvertMapType;
static ConvertMapType &getConvertMap(void)
{
    static Poco::SingletonHolder<ConvertMapType> sh;
    return *sh.get();
}

//! combine two type hashes to form a unique hash such that hash(a, b) != hash(b, a)
static inline size_t typesHashCombine(const std::type_info &inType, const std::type_info &outType)
{
    return inType.hash_code() ^ Poco::hash(outType.hash_code());
}

/***********************************************************************
 * Conversion registration handling
 **********************************************************************/
static void handleConvertPluginEvent(const Pothos::Plugin &plugin, const std::string &event)
{
    poco_information_f2(Poco::Logger::get("Pothos.Object.handleConvertPluginEvent"), "plugin %s, event %s", plugin.toString(), event);
    try
    {
        //validate the plugin -- if we want to handle it -- check the signature:
        if (plugin.getObject().type() != typeid(Pothos::Callable)) return;
        const auto &call = plugin.getObject().extract<Pothos::Callable>();
        if (call.type(-1) == typeid(void)) return;
        if (call.getNumArgs() != 1) return;

        //extract type info used for map lookup
        const std::type_info &inputType = call.type(0);
        const std::type_info &outputType = call.type(-1);

        Poco::RWLock::ScopedWriteLock lock(getMapMutex());
        if (event == "add")
        {
            getConvertMap()[typesHashCombine(inputType, outputType)] = plugin;
        }
        if (event == "remove")
        {
            getConvertMap()[typesHashCombine(inputType, outputType)] = Pothos::Plugin();
        }
    }
    catch(const Pothos::Exception &ex)
    {
        poco_error_f3(Poco::Logger::get("Pothos.Object.handleConvertPluginEvent"),
            "exception %s, plugin %s, event %s", ex.displayText(), plugin.toString(), event);
    }
    catch(...)
    {
        poco_error_f3(Poco::Logger::get("Pothos.Object.handleConvertPluginEvent"),
            "exception %s, plugin %s, event %s", std::string("unknown"), plugin.toString(), event);
    }
}

/***********************************************************************
 * Register event handler
 **********************************************************************/
pothos_static_block(pothosObjectConvertRegister)
{
    Pothos::PluginRegistry::addCall("/object/convert", &handleConvertPluginEvent);
}

/***********************************************************************
 * The conversion implementation
 **********************************************************************/
static Pothos::Object convertObject(const Pothos::Object &inputObj, const std::type_info &outputType)
{
    //find the plugin in the map, it will be null if not found
    Poco::RWLock::ScopedReadLock lock(getMapMutex());
    auto it = getConvertMap().find(typesHashCombine(inputObj.type(), outputType));

    //thow an error when the conversion is not supported
    if (it == getConvertMap().end()) throw Pothos::ObjectConvertError(
        "Pothos::Detail::convert()",
        Poco::format("doesnt support %s to %s",
        inputObj.getTypeString(),
        Pothos::Util::typeInfoToString(outputType)));

    auto call = it->second.getObject().extract<Pothos::Callable>();
    return call.opaqueCall(&inputObj, 1);
}

Pothos::Object Pothos::Object::convert(const std::type_info &type) const
{
    if (this->type() == type) return *this; //type is the same, just copy the Object (efficient)
    if (type == typeid(Pothos::Object)) return Pothos::Object::make(*this);
    return convertObject(*this, type);
}

bool Pothos::Object::canConvert(const std::type_info &type) const
{
    if (type == typeid(Pothos::Object)) return true;
    return Object::canConvert(this->type(), type);
}

bool Pothos::Object::canConvert(const std::type_info &srcType, const std::type_info &dstType)
{
    if (srcType == dstType) return true;
    Poco::RWLock::ScopedReadLock lock(getMapMutex());
    auto it = getConvertMap().find(typesHashCombine(srcType, dstType));
    return (it != getConvertMap().end());
}
