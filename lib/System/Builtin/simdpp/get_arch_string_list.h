/*  Copyright (C) 2015  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_DISPATCH_GET_ARCH_STRING_LIST_H
#define LIBSIMDPP_DISPATCH_GET_ARCH_STRING_LIST_H

#include <vector>
#include <cstring>
#include <string>

//
// #defines copied from simdpp/setup_arch.h
//

#if !defined(SIMDPP_X86) && (__i386__ || __i386 || _M_IX86 || __amd64__ || __x64_64__ || _M_AMD64 || _M_X64)
#define SIMDPP_X86 1
#elif !defined(SIMDPP_ARM) && (_M_ARM || __arm__ || __aarch64__)
#define SIMDPP_ARM 1
#elif !defined(SIMDPP_PPC) && (__powerpc__ || __powerpc64__)
#define SIMDPP_PPC 1
#elif !defined(SIMDPP_MIPS) && __mips__
#define SIMDPP_MIPS 1
#endif

#ifndef SIMDPP_64_BITS
#if __amd64__ || __x86_64__ || _M_AMD64 || __aarch64__ || __powerpc64__
#define SIMDPP_64_BITS 1
#define SIMDPP_32_BITS 0
#else
#define SIMDPP_32_BITS 1
#define SIMDPP_64_BITS 0
#endif
#endif

#include "arch.h"

namespace simdpp {

struct ArchDesc {
    std::string id;
    Arch arch;

    ArchDesc(const std::string& i, Arch a) : id(i), arch(a) {}
};

/** Retrieves supported architecture from given string list. The architecture
    names are the same as ids specified in simdpp/detail/insn_id.h
*/
inline std::vector<ArchDesc> get_architectures()
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
    features.emplace_back("sse4p1", a_sse4_1);
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

    return features;
}

} // namespace simdpp

#endif
