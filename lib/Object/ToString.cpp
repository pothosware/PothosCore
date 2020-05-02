// Copyright (c) 2015-2017 Josh Blum
//                    2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Object/ObjectImpl.hpp>
#include <Pothos/Object/Containers.hpp>
#include <Pothos/Framework/DType.hpp>
#include <Pothos/Callable.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/Util/SpinLockRW.hpp>

#include <Poco/Logger.h>
#include <Poco/Format.h>

#include <complex>
#include <cassert>
#include <functional>
#include <mutex>
#include <map>

/***********************************************************************
 * Global map structure for comparisons
 **********************************************************************/
static Pothos::Util::SpinLockRW &getMapMutex(void)
{
    static Pothos::Util::SpinLockRW lock;
    return lock;
}

//singleton global map for all supported comparisons
typedef std::map<size_t, Pothos::Plugin> ToStringMapType;
static ToStringMapType &getToStringMap(void)
{
    static ToStringMapType map;
    return map;
}

/***********************************************************************
 * ToString registration handling
 **********************************************************************/
static void handleToStringPluginEvent(const Pothos::Plugin &plugin, const std::string &event)
{
    poco_debug_f2(Poco::Logger::get("Pothos.Object.handleToStringPluginEvent"), "plugin %s, event %s", plugin.toString(), event);
    POTHOS_EXCEPTION_TRY
    {
        //validate the plugin -- if we want to handle it -- check the signature:
        if (plugin.getObject().type() != typeid(Pothos::Callable)) return;
        const auto &call = plugin.getObject().extract<Pothos::Callable>();
        if (call.type(-1) != typeid(std::string)) return;
        if (call.getNumArgs() != 1) return;

        //extract type info used for map lookup
        const std::type_info &t = call.type(0);

        std::lock_guard<Pothos::Util::SpinLockRW> lock(getMapMutex());
        if (event == "add")
        {
            getToStringMap()[t.hash_code()] = plugin;
        }
        if (event == "remove")
        {
            getToStringMap()[t.hash_code()] = Pothos::Plugin();
        }
    }
    POTHOS_EXCEPTION_CATCH(const Pothos::Exception &ex)
    {
        poco_error_f3(Poco::Logger::get("Pothos.Object.handleToStringPluginEvent"),
            "exception %s, plugin %s, event %s", ex.displayText(), plugin.toString(), event);
    }
}

/***********************************************************************
 * Register event handler
 **********************************************************************/
pothos_static_block(pothosObjectToStringRegister)
{
    Pothos::PluginRegistry::addCall("/object/tostring", &handleToStringPluginEvent);
}

/***********************************************************************
 * Public-facing function
 **********************************************************************/

std::string Pothos::Object::toString(void) const
{
    //boolean
    if (this->type() == typeid(bool)) return this->extract<bool>()?"true":"false";

    //string
    if (this->type() == typeid(std::string)) return Poco::format("\"%s\"", this->extract<std::string>());

    //try numeric types
    try
    {
        const Pothos::DType dtype(this->type());
        if (dtype.isComplex() and dtype.isFloat())
        {
            const std::complex<double> c = *this;
            if (c.imag() == 0.0) return std::to_string(c.real());
            if (c.real() == 0.0) return std::to_string(c.imag())+"j";
            if (c.imag() < 0.0) return Poco::format("%f-%fj", c.real(), -c.imag());
            return Poco::format("%f+%fj", c.real(), c.imag());
        }
        if (dtype.isComplex())
        {
            const std::complex<long long> c = *this;
            if (c.imag() == 0) return std::to_string(c.real());
            if (c.real() == 0) return std::to_string(c.imag())+"j";
            if (c.imag() < 0) return Poco::format("%s-%sj", std::to_string(c.real()), std::to_string(-c.imag()));
            return Poco::format("%s+%sj", std::to_string(c.real()), std::to_string(c.imag()));
        }
        if (dtype.isFloat()) return std::to_string(double(*this));
        if (dtype.isSigned()) return std::to_string((long long)(*this));
        return std::to_string((unsigned long long)(*this));
    }
    catch (...) {}

    //try object vector
    if (this->canConvert(typeid(Pothos::ObjectVector))) try
    {
        std::string out = "[";
        for (const auto &obj_i : this->convert<Pothos::ObjectVector>())
        {
            if (out.size() > 1) out += ", ";
            out += obj_i.toString();
        }
        return out + "]";
    }
    catch (...) {}

    //try object map
    if (this->canConvert(typeid(Pothos::ObjectMap))) try
    {
        std::string out = "{";
        for (const auto &pair : this->convert<Pothos::ObjectMap>())
        {
            if (out.size() > 1) out += ", ";
            out += Poco::format("%s: %s", pair.first.toString(), pair.second.toString());
        }
        return out + "}";
    }
    catch (...) {}

    //try object set
    if (this->canConvert(typeid(Pothos::ObjectSet))) try
    {
        std::string out = "(";
        for (const auto &obj_i : this->convert<Pothos::ObjectSet>())
        {
            if (out.size() > 1) out += ", ";
            out += obj_i.toString();
        }
        return out + ")";
    }
    catch (...) {}

    //check for a registered toString(), it will be null if not found
    Pothos::Util::SpinLockRW::SharedLock lock(getMapMutex());
    auto it = getToStringMap().find(this->type().hash_code());
    if(getToStringMap().end() != it)
    {
        const auto &call = it->second.getObject().extract<Pothos::Callable>();

        Pothos::Object arg0(*this);
        return call.opaqueCall(&arg0, 1).extract<std::string>();
    }

    //fall-back data type
    return Poco::format("<%s>", this->getTypeString());
}
