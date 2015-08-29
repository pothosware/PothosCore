// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Exception.hpp>
#include <portaudio.h>
#include <cctype>
#include <string>
#include <algorithm>

static inline PaDeviceIndex getDeviceMatch(const std::string blockName, const std::string &deviceName, const PaDeviceIndex defaultIndex)
{
    //empty name, use default
    if (deviceName.empty()) return defaultIndex;

    //numeric name, use index
    if (std::all_of(deviceName.begin(), deviceName.end(), ::isdigit))
    {
        auto index = std::stoi(deviceName);
        if (index >= Pa_GetDeviceCount()) throw Pothos::RangeException(blockName+"("+deviceName+")", "Device index out of range");
        return index;
    }

    //find the match by name
    for (PaDeviceIndex i = 0; i < Pa_GetDeviceCount(); i++)
    {
        if (Pa_GetDeviceInfo(i)->name == deviceName) return i;
    }

    //cant locate by name
    throw Pothos::NotFoundException(blockName+"("+deviceName+")", "No matching device");
}

