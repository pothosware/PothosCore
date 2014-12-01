// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <portaudio.h>

static Poco::JSON::Object::Ptr enumerateAudioDevices(void)
{
    Poco::JSON::Object::Ptr topObject = new Poco::JSON::Object();
    Pa_Initialize();

    Poco::JSON::Array::Ptr devicesArray = new Poco::JSON::Array();
    topObject->set("PortAudio Device", devicesArray);
    for (PaDeviceIndex i = 0; i < Pa_GetDeviceCount(); i++)
    {
        auto info = Pa_GetDeviceInfo(i);
        Poco::JSON::Object::Ptr infoObject = new Poco::JSON::Object();
        infoObject->set("Device Name", std::string(info->name));
        infoObject->set("Host API Name", std::string(Pa_GetHostApiInfo(info->hostApi)->name));
        infoObject->set("Max Input Channels", info->maxInputChannels);
        infoObject->set("Max Output Channels", info->maxOutputChannels);
        infoObject->set("Default Sample Rate", info->defaultSampleRate);
        devicesArray->add(infoObject);
    }

    topObject->set("PortAudio Version", std::string(Pa_GetVersionText()));

    Pa_Terminate();
    return topObject;
}

pothos_static_block(registerAudioInfo)
{
    Pothos::PluginRegistry::addCall(
        "/devices/audio/info", &enumerateAudioDevices);
}
