// Copyright (c) 2014-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include <yaml-cpp/yaml.h>
#include <sstream>

class DeviceInfoUtilsDumpJson
{
public:
    static std::string dump(void)
    {
        YAML::Node deviceObj;
        for (const auto &deviceName : Pothos::PluginRegistry::list("/devices"))
        {
            auto path = Pothos::PluginPath("/devices").join(deviceName).join("info");
            if (not Pothos::PluginRegistry::exists(path)) continue;
            auto plugin = Pothos::PluginRegistry::get(path);
            auto call = plugin.getObject().extract<Pothos::Callable>();
            deviceObj.push_back(YAML::Load(call.call<std::string>()));
        }
        std::stringstream ss;
        ss << deviceObj;
        return ss.str();
    }
};

#include <Pothos/Managed.hpp>

static auto managedDeviceInfoUtils = Pothos::ManagedClass()
    .registerClass<DeviceInfoUtilsDumpJson>()
    .registerStaticMethod("dumpJson", &DeviceInfoUtilsDumpJson::dump)
    .commit("Pothos/Util/DeviceInfoUtils");
