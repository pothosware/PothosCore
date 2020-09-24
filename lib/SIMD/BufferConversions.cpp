// Copyright (c) 2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include "BufferConversions.hpp"

#include <simdpp/simd.h>
#include <simdpp/dispatch/get_arch_gcc_builtin_cpu_supports.h>
#include <simdpp/dispatch/get_arch_linux_cpuinfo.h>
#include <simdpp/dispatch/get_arch_raw_cpuid.h>

//
// This macro needs to be defined.
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

namespace PothosSIMD
{
    // This set of macros is how SIMDPP does its dynamic dispatch.

    SIMDPP_MAKE_DISPATCHER((template<typename InType, typename OutType>)(<InType, OutType>)(void)(bufferConvert)((const void*) in, (void*) out, (size_t) len))

    // TODO: these namespaces don't match what we generate, fix that
    SIMDPP_INSTANTIATE_DISPATCHER(
        (template void bufferConvert<float, double>(const void* in, void* out, size_t len)),
        (template void bufferConvert<double, float>(const void* in, void* out, size_t len))
    )
    
    template <typename InType, typename OutType>
    std::vector<OutType> vectorConvert(const std::vector<InType>& in);

}