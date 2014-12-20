// Copyright (c) 2013-2013 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosUtil.hpp"
#include <Pothos/Plugin.hpp>
#include <Pothos/Proxy.hpp>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <sstream>
#include <iostream>

void PothosUtilBase::printDeviceInfo(const std::string &, const std::string &)
{
    Pothos::init();

    //query the device info as a JSON string
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto deviceInfoStr = env->findProxy("Pothos/Util/DeviceInfoUtils").call<std::string>("dumpJson");

    //parse device info into JSON object
    Poco::JSON::Parser p; p.parse(deviceInfoStr);
    auto infoObj = p.getHandler()->asVar().extract<Poco::JSON::Array::Ptr>();

    //dump back into string with indentation
    std::stringstream ss;
    infoObj->stringify(ss, 4);

    std::cout << ss.str() << std::endl;
}
