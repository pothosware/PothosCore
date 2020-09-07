// Copyright (c) 2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include "DetectAvailableSIMDArchitectures.hpp"

#include <simdpp/simd.h>
#include <simdpp/dispatch/get_arch_gcc_builtin_cpu_supports.h>
#include <simdpp/dispatch/get_arch_linux_cpuinfo.h>
#include <simdpp/dispatch/get_arch_raw_cpuid.h>

#include <algorithm>

//
// Implementation
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

static std::vector<ArchDesc> getAllPotentialArches()
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

    features.emplace_back("sse2", a_sse2);
    features.emplace_back("sse3", a_sse3);
    features.emplace_back("ssse3", a_ssse3);
    features.emplace_back("sse4_1", a_sse4_1);
    features.emplace_back("popcnt", a_popcnt);
    features.emplace_back("avx", a_avx);
    features.emplace_back("avx2", a_avx2);
    features.emplace_back("fma3", a_fma3);
    features.emplace_back("fma4", a_fma4);
    features.emplace_back("xop", a_xop);
    features.emplace_back("avx512f", a_avx512f);
    features.emplace_back("avx512bw", a_avx512bw);
    features.emplace_back("avx512dq", a_avx512dq);
    features.emplace_back("avx512vl", a_avx512vl);
#elif SIMDPP_PPC
    Arch a_altivec = Arch::POWER_ALTIVEC;
    Arch a_vsx_206 = a_altivec | Arch::POWER_VSX_206;
    Arch a_vsx_207 = a_vsx_206 | Arch::POWER_VSX_207;

    features.emplace_back("altivec", a_altivec);
    features.emplace_back("vsx_206", a_vsx_206);
    features.emplace_back("vsx_207", a_vsx_207);
#elif SIMDPP_MIPS
    features.emplace_back("msa", Arch::MIPS_MSA);
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

static std::vector<std::string> _detectAvailableSIMDArchitectures()
{
    const auto simdppArchInfo = SIMDPP_USER_ARCH_INFO;
    const auto arches = getAllPotentialArches();

    std::vector<std::string> simdArchitectures;
    for(const auto& arch: arches)
    {
        if(bool(simdppArchInfo & arch.arch)) simdArchitectures.emplace_back(arch.id);
    }

    return simdArchitectures;
}

std::vector<std::string> Pothos::System::detectAvailableSIMDArchitectures()
{
    // Only do this once
    static const auto availableSIMDArchitectures = _detectAvailableSIMDArchitectures();

    return availableSIMDArchitectures;
}
