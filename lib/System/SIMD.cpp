// Copyright (c) 2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Object.hpp>
#include <Pothos/System/SIMD.hpp>

#include <simdpp/simd.h>
#include <simdpp/dispatch/get_arch_gcc_builtin_cpu_supports.h>
#include <simdpp/dispatch/get_arch_linux_cpuinfo.h>
#include <simdpp/dispatch/get_arch_raw_cpuid.h>

#include <algorithm>
#include <sstream>

//
// Internal libsimdpp code
//

#if SIMDPP_HAS_GET_ARCH_RAW_CPUID
#define SIMDPP_USER_ARCH_INFO simdpp::get_arch_raw_cpuid()
#elif SIMDPP_HAS_GET_ARCH_GCC_BUILTIN_CPU_SUPPORTS
#define SIMDPP_USER_ARCH_INFO simdpp::get_arch_gcc_builtin_cpu_supports()
#elif SIMDPP_HAS_GET_ARCH_LINUX_CPUINFO
#define SIMDPP_USER_ARCH_INFO simdpp::get_arch_linux_cpuinfo()
#else
#define SIMDPP_USER_ARCH_INFO simdpp::Arch::NONE_NULL
#endif

//
// Cannibalized from simdpp::get_arch_string_list internals
//
using simdpp::Arch;

struct ArchDesc
{
    std::string id;
    Arch arch;

    ArchDesc(const std::string& i, Arch a) : id(i), arch(a) {}
};

static std::vector<ArchDesc> getAllPotentialSIMDFeatures()
{
    std::vector<ArchDesc> features;

#if SIMDPP_ARM && SIMDPP_32_BITS
    features.emplace_back("neon", Arch::ARM_NEON);
    features.emplace_back("neonfltsp", Arch::ARM_NEON | Arch::ARM_NEON_FLT_SP);
#elif SIMDPP_ARM && SIMDPP_64_BITS
    features.emplace_back("neon", Arch::ARM_NEON | Arch::ARM_NEON_FLT_SP);
    features.emplace_back("neonfltsp", Arch::ARM_NEON | Arch::ARM_NEON_FLT_SP);
#elif SIMDPP_X86
    Arch a_sse2 = Arch::X86_SSE2;
    Arch a_sse3 = a_sse2 | Arch::X86_SSE3;
    Arch a_ssse3 = a_sse3 | Arch::X86_SSSE3;
    Arch a_sse4_1 = a_ssse3 | Arch::X86_SSE4_1;
    Arch a_popcnt = Arch::X86_POPCNT_INSN;
    Arch a_avx = a_sse4_1 | Arch::X86_AVX;
    Arch a_avx2 = a_avx | Arch::X86_AVX2;
    Arch a_fma3 = a_sse3 | Arch::X86_FMA3;
    Arch a_fma4 = a_sse3 | Arch::X86_FMA4;
    Arch a_xop = a_sse3 | Arch::X86_XOP;
    Arch a_avx512f = a_avx2 | Arch::X86_AVX512F;
    Arch a_avx512bw = a_avx512f | Arch::X86_AVX512BW;
    Arch a_avx512dq = a_avx512f | Arch::X86_AVX512DQ;
    Arch a_avx512vl = a_avx512f | Arch::X86_AVX512VL;

    features.emplace_back("x86_sse2", a_sse2);
    features.emplace_back("x86_sse3", a_sse3);
    features.emplace_back("x86_ssse3", a_ssse3);
    features.emplace_back("x86_sse4_1", a_sse4_1);
    features.emplace_back("x86_popcnt", a_popcnt);
    features.emplace_back("x86_avx", a_avx);
    features.emplace_back("x86_avx2", a_avx2);
    features.emplace_back("x86_fma3", a_fma3);
    features.emplace_back("x86_fma4", a_fma4);
    features.emplace_back("x86_xop", a_xop);
    features.emplace_back("x86_avx512f", a_avx512f);
    features.emplace_back("x86_avx512bw", a_avx512bw);
    features.emplace_back("x86_avx512dq", a_avx512dq);
    features.emplace_back("x86_avx512vl", a_avx512vl);
#elif SIMDPP_PPC
    Arch a_altivec = Arch::POWER_ALTIVEC;
    Arch a_vsx_206 = a_altivec | Arch::POWER_VSX_206;
    Arch a_vsx_207 = a_vsx_206 | Arch::POWER_VSX_207;

    features.emplace_back("power_altivec", a_altivec);
    features.emplace_back("power_vsx_206", a_vsx_206);
    features.emplace_back("power_vsx_207", a_vsx_207);
#elif SIMDPP_MIPS
    features.emplace_back("mips_msa", Arch::MIPS_MSA);
#endif

    // Per the simdpp::Arch documentation, the higher the arch enum,
    // the faster the instruction set is likely to be.
    std::sort(
        features.begin(),
        features.end(),
        [](const ArchDesc& archDesc0, const ArchDesc& archDesc1)
        {
            return (archDesc0.arch > archDesc1.arch);
        });

    return features;
}

//
// Pothos-level utility code
//

static const std::string SEPARATOR = "__";

static std::vector<std::string> _getSupportedSIMDFeatureSet()
{
    const auto simdppArchInfo = SIMDPP_USER_ARCH_INFO;
    const auto arches = getAllPotentialSIMDFeatures();

    std::vector<std::string> simdArchitectures;
    for(const auto& arch: arches)
    {
        if(simdpp::test_arch_subset(simdppArchInfo, arch.arch)) simdArchitectures.emplace_back(arch.id);
    }

    return simdArchitectures;
}

static std::vector<std::string> _separateSIMDFeatureSetKey(const std::string& featureSetKey)
{
    // "x86_sse2__x86_sse3__x86_ssse3" -> {"x86_sse2","x86_sse3","x86_ssse3"}
    std::string keyCopy(featureSetKey);
    std::vector<std::string> separatedKey;

    size_t pos = 0;
    std::string token;
    while ((pos = keyCopy.find(SEPARATOR)) != std::string::npos)
    {
        token = keyCopy.substr(0, pos);
        separatedKey.emplace_back(token);
        keyCopy.erase(0, pos + SEPARATOR.length());
    }
    separatedKey.emplace_back(keyCopy); // last entry

    return separatedKey;
}

static bool _isSIMDFeatureSetSupported(const std::vector<std::string>& featureSet)
{
    const auto supportedSIMDFeatureSet = Pothos::System::getSupportedSIMDFeatureSet();

    auto unsupportedFeatureIter = std::find_if(
                                      featureSet.begin(),
                                      featureSet.end(),
                                      [&](const std::string& feature)
                                      {
                                          auto supportedFeatureIter =
                                                   std::find(
                                                       supportedSIMDFeatureSet.begin(),
                                                       supportedSIMDFeatureSet.end(),
                                                       feature);

                                          return (supportedFeatureIter == supportedSIMDFeatureSet.end());
                                      });

    return (unsupportedFeatureIter == featureSet.end());
}

static bool _isSIMDFeatureSetSupported(const std::string& featureSetKey)
{
    return _isSIMDFeatureSetSupported(_separateSIMDFeatureSetKey(featureSetKey));
}

//
// Exported functions
//

std::vector<std::string> Pothos::System::getSupportedSIMDFeatureSet()
{
    // Only do this once
    static const auto supportedSIMDFeatureSet = _getSupportedSIMDFeatureSet();

    return supportedSIMDFeatureSet;
}

POTHOS_API std::string Pothos::System::getOptimalSIMDFeatureSetKey(const std::vector<std::string>& featureSetKeys)
{
    std::vector<std::string> keys;
    std::copy_if(
        featureSetKeys.begin(),
        featureSetKeys.end(),
        std::back_inserter(keys),
        [](const std::string& key)
        {
            return _isSIMDFeatureSetSupported(key);
        });
    if(keys.empty()) return "fallback";
    else             return *(keys.end()-1); // Sorted in ascending order of performance
}
