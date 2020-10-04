// Copyright (c) 2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include "BufferConversions.hpp"

#include <Pothos/Exception.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/System/SIMD.hpp>

#include <Poco/Format.h>

#include <iostream>

namespace PothosSIMD
{
    static BufferConvertFcnMap getOptimalBufferConvertMap()
    {
        const std::string bufferConverterPluginPath = "/simd/buffer_converter_maps";
        const auto simdArches = Pothos::PluginRegistry::list(bufferConverterPluginPath);

        auto optimalArch = Pothos::System::getOptimalSIMDFeatureSetKey(simdArches);

        return Pothos::PluginRegistry::get(bufferConverterPluginPath + "/" + optimalArch).getObject();
    }

    BufferConvertFcn bufferConvertDispatch(
        const Pothos::DType& inType,
        const Pothos::DType& outType)
    {
        static const auto optimalBufferConvertMap = getOptimalBufferConvertMap();
        auto dtypeHash = dtypeIOToHash(inType, outType);

        auto iter = optimalBufferConvertMap.find(dtypeHash);
        if (iter != optimalBufferConvertMap.end()) return iter->second;
        else
        {
            throw Pothos::AssertionViolationException(
                      Poco::format(
                          "No SIMD buffer converter registered for %s -> %s",
                          inType.toString(),
                          outType.toString()));
        }
    }
}