// Copyright (c) 2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include "PothosUtil.hpp"

#include <Pothos/Plugin.hpp>
#include <Pothos/Proxy.hpp>

#include <Poco/File.h>
#include <Poco/StringTokenizer.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

/***********************************************************************
 * Generate a C++ header with SIMD dispatchers for a given Pothos module
 **********************************************************************/
void PothosUtilBase::generateSIMDDispatchers(const std::string&, const std::string& jsonPath)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto simdDispatcher = env->findProxy("Pothos/Util/SIMDDispatcher");

    const auto simdArchesStr = this->config().getString("simdArches");
    const auto outputFilePath = this->config().getString("outputFile");

    Poco::StringTokenizer simdArchesTokenizer(simdArchesStr, ",", Poco::StringTokenizer::TOK_TRIM);
    std::vector<std::string> simdArches(simdArchesTokenizer.begin(), simdArchesTokenizer.end());

    const auto fileSize = Poco::File(jsonPath).getSize();

    std::ifstream ifile(jsonPath.c_str(), std::ios::in);
    std::string jsonInput;
    jsonInput.resize(fileSize);
    ifile.read((char*)jsonInput.data(), fileSize);

    const auto outputHeaderString = simdDispatcher.call<std::string>("generateAndDumpHeader", jsonInput, simdArches);

    std::ofstream ofile(outputFilePath.c_str(), std::ios::out);
    ofile << outputHeaderString;
}