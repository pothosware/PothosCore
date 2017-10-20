// Copyright (c) 2013-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Object/ObjectImpl.hpp>
#include <Pothos/Object/Exception.hpp>
#include <Pothos/Util/SpinLockRW.hpp>
#include <Pothos/Util/TypeInfo.hpp>
#include <Pothos/Callable.hpp>
#include <Pothos/Plugin.hpp>
#include <Poco/Logger.h>
#include <Poco/Format.h>
#include <Poco/Hash.h>
#include <mutex>
#include <set>
#include <map>

/***********************************************************************
 * Global map structure for conversions
 **********************************************************************/
static Pothos::Util::SpinLockRW &getMapMutex(void)
{
    static Pothos::Util::SpinLockRW lock;
    return lock;
}

//singleton global map for all supported conversions
typedef std::map<size_t, Pothos::Plugin> ConvertMapType;
static ConvertMapType &getConvertMap(void)
{
    static ConvertMapType map;
    return map;
}

//given an input type (hash), what types can it can convert to?
static std::map<size_t, std::set<size_t>> &getConvertIoMap(void)
{
    static std::map<size_t, std::set<size_t>> map;
    return map;
}

//! combine two type hashes to form a unique hash such that hash(a, b) != hash(b, a)
static inline size_t typesHashCombine(const size_t &inTypeHash, const size_t &outTypeHash)
{
    return inTypeHash ^ Poco::hash(outTypeHash);
}

static inline size_t typesHashCombine(const std::type_info &inType, const std::type_info &outType)
{
    return typesHashCombine(inType.hash_code(), outType.hash_code());
}

/***********************************************************************
 * Conversion registration handling
 **********************************************************************/
static void handleConvertPluginEvent(const Pothos::Plugin &plugin, const std::string &event)
{
    poco_debug_f2(Poco::Logger::get("Pothos.Object.handleConvertPluginEvent"), "plugin %s, event %s", plugin.toString(), event);
    POTHOS_EXCEPTION_TRY
    {
        //validate the plugin -- if we want to handle it -- check the signature:
        if (plugin.getObject().type() != typeid(Pothos::Callable)) return;
        const auto &call = plugin.getObject().extract<Pothos::Callable>();
        if (call.type(-1) == typeid(void)) return;
        if (call.getNumArgs() != 1) return;

        //extract type info used for map lookup
        const std::type_info &inputType = call.type(0);
        const std::type_info &outputType = call.type(-1);

        std::lock_guard<Pothos::Util::SpinLockRW> lock(getMapMutex());
        if (event == "add")
        {
            getConvertMap()[typesHashCombine(inputType, outputType)] = plugin;
            getConvertIoMap()[inputType.hash_code()].insert(outputType.hash_code());
        }
        if (event == "remove")
        {
            getConvertMap()[typesHashCombine(inputType, outputType)] = Pothos::Plugin();
            getConvertIoMap()[inputType.hash_code()].erase(outputType.hash_code());
        }
    }
    POTHOS_EXCEPTION_CATCH(const Pothos::Exception &ex)
    {
        poco_error_f3(Poco::Logger::get("Pothos.Object.handleConvertPluginEvent"),
            "exception %s, plugin %s, event %s", ex.displayText(), plugin.toString(), event);
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
    Pothos::Util::SpinLockRW::SharedLock lock(getMapMutex());
    auto it = getConvertMap().find(typesHashCombine(inputObj.type(), outputType));

    //try an intermediate conversion
    if (it == getConvertMap().end())
    {
        auto itIo = getConvertIoMap().find(inputObj.type().hash_code());
        if (itIo != getConvertIoMap().end()) for (const size_t intermHash : itIo->second)
        {
            auto it1 = getConvertMap().find(typesHashCombine(inputObj.type().hash_code(), intermHash));
            auto it2 = getConvertMap().find(typesHashCombine(intermHash, outputType.hash_code()));
            if (it1 != getConvertMap().end() and it2 != getConvertMap().end())
            {
                auto call1 = it1->second.getObject().extract<Pothos::Callable>();
                auto call2 = it2->second.getObject().extract<Pothos::Callable>();
                Pothos::Object intermediate = call1.opaqueCall(&inputObj, 1);
                return call2.opaqueCall(&intermediate, 1);
            }
        }
    }

    //thow an error when the conversion is not supported
    if (it == getConvertMap().end()) throw Pothos::ObjectConvertError(
        "Pothos::Object::convert()",
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
    Pothos::Util::SpinLockRW::SharedLock lock(getMapMutex());
    auto it = getConvertMap().find(typesHashCombine(srcType, dstType));

    //try an intermediate conversion
    if (it == getConvertMap().end())
    {
        auto itIo = getConvertIoMap().find(srcType.hash_code());
        if (itIo != getConvertIoMap().end()) for (const size_t intermHash : itIo->second)
        {
            auto it1 = getConvertMap().find(typesHashCombine(srcType.hash_code(), intermHash));
            auto it2 = getConvertMap().find(typesHashCombine(intermHash, dstType.hash_code()));
            if (it1 != getConvertMap().end() and it2 != getConvertMap().end()) return true;
        }
    }

    return (it != getConvertMap().end());
}
