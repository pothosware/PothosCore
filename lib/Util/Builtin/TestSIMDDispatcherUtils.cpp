// Copyright (c) 2020-2021 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Proxy.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/System.hpp>
#include <Pothos/Testing.hpp>
#include <Pothos/Util/Compiler.hpp>

#include "json.hpp"

#include <Poco/Format.h>

#include <fstream>
#include <iostream>
#include <string>

static std::string join(
    const std::vector<std::string>& stringVec,
    const std::string& delimiter)
{
    std::stringstream sstream;
    for (const auto& str : stringVec)
    {
        if (&str != &stringVec[0]) sstream << delimiter;
        sstream << str;
    }

    return sstream.str();
}

POTHOS_TEST_BLOCK("/util/tests", test_simd_dispatcher_utils_dump_header)
{
    const auto jsonInputString =
        "{"
        "    \"namespace\": \"TestNamespace\","
        "    \"functions\":"
        "    ["
        "        {"
        "            \"name\": \"clamp\","
        "            \"returnType\": \"void\","
        "            \"params\": [\"const float*\", \"float*\", \"float\", \"float\", \"size_t\"]"
        "        },"
        "        {"
        "            \"name\": \"minmax\","
        "            \"returnType\": \"void\","
        "            \"paramTypes\": [\"T\"],"
        "            \"params\": [\"const T**\", \"T*\", \"T*\", \"size_t\", \"size_t\"]"
        "        },"
        "        {"
        "            \"name\": \"greaterThan\","
        "            \"returnType\": \"void\","
        "            \"paramTypes\": [\"T\"],"
        "            \"params\": [\"const T*\", \"const T*\", \"char*\", \"size_t\"]"
        "        },"
        "        {"
        "            \"name\": \"bufferConvert\","
        "            \"returnType\": \"void\","
        "            \"paramTypes\": [\"InType\", \"OutType\"],"
        "            \"params\": [\"const InType*\", \"OutType*\", \"size_t\"]"
        "        },"
        "        {"
        "            \"name\": \"vectorConvert\","
        "            \"returnType\": \"std::vector<OutType>\","
        "            \"paramTypes\": [\"InType\", \"OutType\"],"
        "            \"params\": [\"const std::vector<InType>&\"]"
        "        }"
        "    ]"
        "}";

    auto env = Pothos::ProxyEnvironment::make("managed");
    auto proxy = env->findProxy("Pothos/Util/SIMDDispatcher");

    auto testFeatureSet = Pothos::System::getSupportedSIMDFeatureSet();
    testFeatureSet.emplace_back(join(testFeatureSet, "__"));

    // Before doing anything, do a sanity check and make sure we're
    // passing in valid JSON.
    std::cout << "Input: " << std::endl << std::endl
              << nlohmann::json::parse(jsonInputString) << std::endl << std::endl;

    auto generatedHeaderString = proxy.call<std::string>(
                                     "generateAndDumpHeader",
                                     jsonInputString,
                                     testFeatureSet);

    // Create stubs for the non-templated function to avoid a linker error.
    for (const auto& arch : testFeatureSet)
    {
        generatedHeaderString += Poco::format("\nvoid TestNamespace::%s::clamp(const float*, float*, float, float, size_t){};", arch);
    }
    generatedHeaderString += "\nvoid TestNamespace::fallback::clamp(const float*, float*, float, float, size_t){};";

    std::cout << "Output: " << std::endl << std::endl << generatedHeaderString
              << std::endl << std::endl;

    // Make sure the generated header is proper C++ syntax.
    const auto compiler = Pothos::Util::Compiler::make();
    POTHOS_TEST_TRUE(compiler->test());

    const auto sourcePath = compiler->createTempFile(".cpp");
    std::ofstream outFile(sourcePath);
    outFile << generatedHeaderString << std::endl;
    outFile.close();

    std::cout << "Testing C++ syntax of generated header..." << std::endl;

    auto args = Pothos::Util::CompilerArgs::defaultDevEnv();
    args.sources.push_back(sourcePath);
    auto out = compiler->compileCppModule(args);
}
