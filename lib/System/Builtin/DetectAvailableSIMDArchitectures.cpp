// Copyright (c) 2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include "simdpp/arch.h"
#include "simdpp/get_arch_gcc_builtin_cpu_supports.h"
#include "simdpp/get_arch_linux_cpuinfo.h"
#include "simdpp/get_arch_raw_cpuid.h"
#include "simdpp/get_arch_string_list.h"

#include "DetectAvailableSIMDArchitectures.hpp"

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
#error simdpp::Arch::NONE_NULL
#endif

std::vector<std::string> Pothos::System::detectAvailableSIMDArchitectures()
{
    const auto simdppArchInfo = SIMDPP_USER_ARCH_INFO;
    const auto arches = simdpp::get_architectures();

    std::vector<std::string> simdArchitectures;
    for(const auto& arch: arches)
    {
        if(bool(simdppArchInfo & arch.arch)) simdArchitectures.emplace_back(arch.id);
    }

    return simdArchitectures;
}
