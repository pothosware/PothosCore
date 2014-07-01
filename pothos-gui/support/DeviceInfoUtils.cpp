// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>
#include <sstream>

class DeviceInfoUtilsDumpJson
{
public:
    static std::string dump(void)
    {
        Poco::JSON::Object::Ptr deviceObj = new Poco::JSON::Object();
        for (const auto &deviceName : Pothos::PluginRegistry::list("/devices"))
        {
            auto path = Pothos::PluginPath("/devices").join(deviceName).join("info");
            if (not Pothos::PluginRegistry::exists(path)) continue;
            auto plugin = Pothos::PluginRegistry::get(path);
            auto call = plugin.getObject().extract<Pothos::Callable>();
            deviceObj->set(deviceName, call.call<Poco::JSON::Object::Ptr>());
        }
        std::stringstream ss;
        deviceObj->stringify(ss);
        return ss.str();
    }
};

#include <Pothos/Managed.hpp>

static auto managedDocUtils = Pothos::ManagedClass()
    .registerClass<DeviceInfoUtilsDumpJson>()
    .registerStaticMethod("dumpJson", &DeviceInfoUtilsDumpJson::dump)
    .commit("Pothos/Gui/DeviceInfoUtils");
