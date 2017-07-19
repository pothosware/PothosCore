// Copyright (c) 2013-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosUtil.hpp"
#include <Pothos/Plugin.hpp>
#include <Pothos/Proxy.hpp>
#include <Poco/Path.h>
#include <sstream>
#include <fstream>
#include <iostream>

void PothosUtilBase::printDeviceInfo(void)
{
    Pothos::ScopedInit init;

    const auto deviceType = this->config().getString("deviceType");
    if (deviceType.empty())
    {
        std::cout << std::endl;
        std::cout << ">>> Specify --device-info=deviceType for more information..." << std::endl;
        std::cout << ">>> Available device types in the plugin tree are: " << std::endl;
        for (const auto &name : Pothos::PluginRegistry::list("/devices"))
        {
            std::cout << "  * " << name << std::endl;
        }
        std::cout << std::endl;
        return;
    }

    //extract the JSON device information
    auto path = Pothos::PluginPath("/devices").join(deviceType).join("info");
    std::cout << ">>> Querying device info: " << path.toString() << std::endl;
    auto plugin = Pothos::PluginRegistry::get(path);
    auto call = plugin.getObject().extract<Pothos::Callable>();
    auto info = call.call<std::string>();

    //dump the information to file
    if (this->config().has("outputFile"))
    {
        const auto infoFile = this->config().getString("outputFile");
        std::cout << ">>> Dumping info: " << infoFile << std::endl;
        std::ofstream ofs(Poco::Path::expand(infoFile));
        ofs << info;
    }
    //or otherwise to stdout
    else
    {
        std::cout << info << std::endl;
    }
}
