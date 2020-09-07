// Copyright (c) 2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Object.hpp>
#include <Pothos/System/SIMD.hpp>
#include <Pothos/Testing.hpp>

#include <algorithm>
#include <iostream>
#include <sstream>

using namespace Pothos::System;

POTHOS_TEST_BLOCK("/system/tests", test_feature_set_key)
{
    const std::vector<std::string> testVector = {"x86_sse2", "x86_sse3", "x86_ssse3"};
    const std::string expectedKey = "x86_sse2__x86_sse3__x86_ssse3";

    POTHOS_TEST_EQUAL(expectedKey, getSIMDFeatureSetKey(testVector));

    // Separating it again should match.
    POTHOS_TEST_EQUAL(testVector, separateSIMDFeatureSetKey(expectedKey));
}

POTHOS_TEST_BLOCK("/system/tests", test_feature_set_support_detection)
{
    const auto supportedSIMDFeatureSet = getSupportedSIMDFeatureSet();
    POTHOS_TEST_FALSE(supportedSIMDFeatureSet.empty());
    std::cout << Pothos::Object(supportedSIMDFeatureSet).toString() << std::endl;

    // Each individual feature should be supported.
    for(const auto& feature: supportedSIMDFeatureSet)
    {
        std::cout << " * Testing \"" << feature << "\" support..." << std::endl;
        POTHOS_TEST_TRUE(isSIMDFeatureSetSupported(feature));

        auto featureVec = std::vector<std::string>{feature};
        std::cout << " * Testing " << Pothos::Object(featureVec).toString() << " support..." << std::endl;
        POTHOS_TEST_TRUE(isSIMDFeatureSetSupported(std::vector<std::string>{feature}));
    }

    // The full set should also be supported.
    const auto fullFeatureSetKey = getSIMDFeatureSetKey(supportedSIMDFeatureSet);

    std::cout << " * Testing \"" << fullFeatureSetKey << "\" support..." << std::endl;
    POTHOS_TEST_TRUE(isSIMDFeatureSetSupported(getSIMDFeatureSetKey(supportedSIMDFeatureSet)));

    std::cout << Pothos::Object(supportedSIMDFeatureSet).toString() << std::endl;
    POTHOS_TEST_TRUE(isSIMDFeatureSetSupported(supportedSIMDFeatureSet));
}
