// Copyright (c) 2014-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include <json.hpp>

using json = nlohmann::json;

class DeviceInfoUtilsDumpJson
{
public:
    static std::string dump(void)
    {
        json deviceObj;
        for (const auto &deviceName : Pothos::PluginRegistry::list("/devices"))
        {
            auto path = Pothos::PluginPath("/devices").join(deviceName).join("info");
            if (not Pothos::PluginRegistry::exists(path)) continue;
            const auto plugin = Pothos::PluginRegistry::get(path);
            const auto &call = plugin.getObject().extract<Pothos::Callable>();
            deviceObj.push_back(json::parse(call.call<std::string>()));
        }
        return deviceObj.dump();
    }
};

#include <Pothos/Managed.hpp>

static auto managedDeviceInfoUtils = Pothos::ManagedClass()
    .registerClass<DeviceInfoUtilsDumpJson>()
    .registerStaticMethod("dumpJson", &DeviceInfoUtilsDumpJson::dump)
    .commit("Pothos/Util/DeviceInfoUtils");
