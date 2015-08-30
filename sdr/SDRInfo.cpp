// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <SoapySDR/Version.hpp>
#include <SoapySDR/Modules.hpp>
#include <SoapySDR/Registry.hpp>
#include <SoapySDR/Device.hpp>

static Poco::JSON::Object::Ptr enumerateSDRDevices(void)
{
    Poco::JSON::Object::Ptr topObject = new Poco::JSON::Object();
    Poco::JSON::Object::Ptr infoObject = new Poco::JSON::Object();
    topObject->set("SoapySDR info", infoObject);

    //install info
    infoObject->set("API Version", SoapySDR::getAPIVersion());
    infoObject->set("ABI Version", SoapySDR::getABIVersion());
    infoObject->set("Install Root", SoapySDR::getRootPath());

    //list of device factories
    SoapySDR::loadModules();
    std::string factories;
    for (const auto &factory : SoapySDR::Registry::listFindFunctions())
    {
        if (not factories.empty()) factories += ", ";
        factories += factory.first;
    }
    infoObject->set("Factories", factories);

    //available devices
    Poco::JSON::Array::Ptr devicesArray = new Poco::JSON::Array();
    topObject->set("SDR Device", devicesArray);
    for (const auto &result : SoapySDR::Device::enumerate())
    {
        Poco::JSON::Object::Ptr deviceObject = new Poco::JSON::Object();
        for (const auto &kwarg : result)
        {
            deviceObject->set(kwarg.first, kwarg.second);
        }
        devicesArray->add(deviceObject);
    }

    return topObject;
}

pothos_static_block(registerSDRInfo)
{
    Pothos::PluginRegistry::addCall(
        "/devices/sdr/info", &enumerateSDRDevices);
}
