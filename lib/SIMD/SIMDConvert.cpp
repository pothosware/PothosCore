// Copyright (c) 2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include "SIMDConvert.hpp"

#include <Pothos/Framework.hpp>
#include <Pothos/Util/Templates.hpp>
#include <Pothos/Util/TypeInfo.hpp>

#include <simdpp/simd.h>

#include <simdpp/dispatch/get_arch_gcc_builtin_cpu_supports.h>
#include <simdpp/dispatch/get_arch_raw_cpuid.h>
#include <simdpp/dispatch/get_arch_linux_cpuinfo.h>

#include <complex>
#include <cstring>
#include <type_traits>

// SIMDPP_USER_ARCH_INFO used by SIMDPP_MAKE_DISPATCHER below
#if SIMDPP_HAS_GET_ARCH_RAW_CPUID
#define SIMDPP_USER_ARCH_INFO ::simdpp::get_arch_raw_cpuid()
#elif SIMDPP_HAS_GET_ARCH_GCC_BUILTIN_CPU_SUPPORTS
#define SIMDPP_USER_ARCH_INFO ::simdpp::get_arch_gcc_builtin_cpu_supports()
#elif SIMDPP_HAS_GET_ARCH_LINUX_CPUINFO
#define SIMDPP_USER_ARCH_INFO ::simdpp::get_arch_linux_cpuinfo()
#else
#error "Unsupported platform"
#endif

namespace SIMDPP_ARCH_NAMESPACE {

namespace detail
{
    /*
     * Make SFINAE structs out of arch-dependent #defines. Each copy
     * of this file will be compiled with different compiler flags,
     * so this SFINAE will behave as needed for each arch.
     */
    template <typename T>
    struct SIMDTraits
    {
        static constexpr bool Supported = false;

        template <typename OutType>
        static constexpr bool canConvertTo() {return false;}
    };

    template <>
    struct SIMDTraits<char>
    {
        static constexpr size_t FastSize = SIMDPP_FAST_INT8_SIZE;

        template <size_t N>
        using SIMDPPType = simdpp::int8<N>;

        using FastType = SIMDPPType<FastSize>;

        // Depends on instruction set
        static constexpr bool Supported = SIMDPP_HAS_INT8_SIMD;

        template <typename OutType>
        static constexpr bool canConvertTo() {return true;}

        template <typename InType>
        static SIMDPPType<SIMDTraits<InType>::FastSize> convertFromFastType(typename SIMDTraits<InType>::FastType inReg)
        {
            simdpp::int8<SIMDTraits<InType>::FastSize> outReg = simdpp::to_int8(inReg);
            return outReg;
        }
    };

    template <>
    struct SIMDTraits<int8_t>
    {
        static constexpr size_t FastSize = SIMDPP_FAST_INT8_SIZE;

        template <size_t N>
        using SIMDPPType = simdpp::int8<N>;

        using FastType = SIMDPPType<FastSize>;

        // Depends on instruction set
        static constexpr bool Supported = SIMDPP_HAS_INT8_SIMD;

        template <typename OutType>
        static constexpr bool canConvertTo() {return true;}

        template <typename InType>
        static SIMDPPType<SIMDTraits<InType>::FastSize> convertFromFastType(typename SIMDTraits<InType>::FastType inReg)
        {
            simdpp::int8<SIMDTraits<InType>::FastSize> outReg = simdpp::to_int8(inReg);
            return outReg;
        }
    };

    template <>
    struct SIMDTraits<int16_t>
    {
        static constexpr size_t FastSize = SIMDPP_FAST_INT16_SIZE;

        template <size_t N>
        using SIMDPPType = simdpp::int16<N>;

        using FastType = SIMDPPType<FastSize>;

        // Depends on instruction set
        static constexpr bool Supported = SIMDPP_HAS_INT16_SIMD;

        template <typename OutType>
        static constexpr bool canConvertTo() {return true;}

        template <typename InType>
        static SIMDPPType<SIMDTraits<InType>::FastSize> convertFromFastType(typename SIMDTraits<InType>::FastType inReg)
        {
            simdpp::int16<SIMDTraits<InType>::FastSize> outReg = simdpp::to_int16(inReg);
            return outReg;
        }
    };

    template <>
    struct SIMDTraits<int32_t>
    {
        static constexpr size_t FastSize = SIMDPP_FAST_INT32_SIZE;

        template <size_t N>
        using SIMDPPType = simdpp::int32<N>;

        using FastType = SIMDPPType<FastSize>;

        // Depends on instruction set
        static constexpr bool Supported = SIMDPP_HAS_INT32_SIMD;

        template <typename OutType>
        static constexpr bool canConvertTo() {return true;}

        template <typename InType>
        static SIMDPPType<SIMDTraits<InType>::FastSize> convertFromFastType(typename SIMDTraits<InType>::FastType inReg)
        {
            simdpp::int32<SIMDTraits<InType>::FastSize> outReg = simdpp::to_int32(inReg);
            return outReg;
        }
    };

    // Buggy
    template <>
    constexpr bool SIMDTraits<int32_t>::canConvertTo<char>() {return false;}

    // Buggy
    template <>
    constexpr bool SIMDTraits<int32_t>::canConvertTo<signed char>() {return false;}

    // Buggy
    template <>
    constexpr bool SIMDTraits<int32_t>::canConvertTo<unsigned char>() {return false;}

    template <>
    struct SIMDTraits<long>
    {
        static constexpr size_t FastSize = SIMDPP_FAST_INT64_SIZE;

        template <size_t N>
        using SIMDPPType = simdpp::int64<N>;

        using FastType = SIMDPPType<FastSize>;

        // Depends on instruction set
        static constexpr bool Supported = SIMDPP_HAS_INT64_SIMD;

        template <typename OutType>
        static constexpr bool canConvertTo() {return false;}

        template <typename InType>
        static SIMDPPType<SIMDTraits<InType>::FastSize> convertFromFastType(typename SIMDTraits<InType>::FastType inReg)
        {
            simdpp::int64<SIMDTraits<InType>::FastSize> outReg = simdpp::to_int64(inReg);
            return outReg;
        }
    };

    // Depends on instruction set
    template <>
    constexpr bool SIMDTraits<long>::canConvertTo<float>() {return bool(SIMDPP_HAS_INT64_TO_FLOAT32_CONVERSION);}

    // Depends on instruction set
    template <>
    constexpr bool SIMDTraits<long>::canConvertTo<double>() {return bool(SIMDPP_HAS_INT64_TO_FLOAT64_CONVERSION);}

    template <>
    struct SIMDTraits<long long>
    {
        static constexpr size_t FastSize = SIMDPP_FAST_INT64_SIZE;

        template <size_t N>
        using SIMDPPType = simdpp::int64<N>;

        using FastType = SIMDPPType<FastSize>;

        // Depends on instruction set
        static constexpr bool Supported = SIMDPP_HAS_INT64_SIMD;

        template <typename OutType>
        static constexpr bool canConvertTo() {return SIMDTraits<long>::canConvertTo<OutType>();}

        template <typename InType>
        static SIMDPPType<SIMDTraits<InType>::FastSize> convertFromFastType(typename SIMDTraits<InType>::FastType inReg)
        {
            simdpp::int64<SIMDTraits<InType>::FastSize> outReg = simdpp::to_int64(inReg);
            return outReg;
        }
    };

    template <>
    struct SIMDTraits<uint8_t>
    {
        static constexpr size_t FastSize = SIMDPP_FAST_INT8_SIZE;

        template <size_t N>
        using SIMDPPType = simdpp::uint8<N>;

        using FastType = SIMDPPType<FastSize>;

        // Depends on instruction set
        static constexpr bool Supported = SIMDPP_HAS_INT8_SIMD;

        template <typename OutType>
        static constexpr bool canConvertTo() {return true;}

        template <typename InType>
        static SIMDPPType<SIMDTraits<InType>::FastSize> convertFromFastType(typename SIMDTraits<InType>::FastType inReg)
        {
            simdpp::uint8<SIMDTraits<InType>::FastSize> outReg = simdpp::to_uint8(inReg);
            return outReg;
        }
    };

    template <>
    struct SIMDTraits<uint16_t>
    {
        static constexpr size_t FastSize = SIMDPP_FAST_INT16_SIZE;

        template <size_t N>
        using SIMDPPType = simdpp::uint16<N>;

        using FastType = SIMDPPType<FastSize>;

        // Depends on instruction set
        static constexpr bool Supported = SIMDPP_HAS_INT16_SIMD;

        template <typename OutType>
        static constexpr bool canConvertTo() {return true;}

        template <typename InType>
        static SIMDPPType<SIMDTraits<InType>::FastSize> convertFromFastType(typename SIMDTraits<InType>::FastType inReg)
        {
            simdpp::uint16<SIMDTraits<InType>::FastSize> outReg = simdpp::to_uint16(inReg);
            return outReg;
        }
    };

    template <>
    struct SIMDTraits<uint32_t>
    {
        static constexpr size_t FastSize = SIMDPP_FAST_INT32_SIZE;

        template <size_t N>
        using SIMDPPType = simdpp::uint32<N>;

        using FastType = SIMDPPType<FastSize>;

        // Depends on instruction set
        static constexpr bool Supported = SIMDPP_HAS_INT32_SIMD;

        template <typename OutType>
        static constexpr bool canConvertTo() {return true;}

        template <typename InType>
        static SIMDPPType<SIMDTraits<InType>::FastSize> convertFromFastType(typename SIMDTraits<InType>::FastType inReg)
        {
            simdpp::uint32<SIMDTraits<InType>::FastSize> outReg = simdpp::to_uint32(inReg);
            return outReg;
        }
    };

    // Buggy
    template <>
    constexpr bool SIMDTraits<uint32_t>::canConvertTo<char>() {return false;}

    // Buggy
    template <>
    constexpr bool SIMDTraits<uint32_t>::canConvertTo<signed char>() {return false;}

    // Buggy
    template <>
    constexpr bool SIMDTraits<uint32_t>::canConvertTo<unsigned char>() {return false;}

    template <>
    struct SIMDTraits<unsigned long>
    {
        static constexpr size_t FastSize = SIMDPP_FAST_INT64_SIZE;

        template <size_t N>
        using SIMDPPType = simdpp::uint64<N>;

        using FastType = SIMDPPType<FastSize>;

        // Depends on instruction set
        static constexpr bool Supported = SIMDPP_HAS_INT64_SIMD;

        template <typename OutType>
        static constexpr bool canConvertTo() {return false;}

        template <typename InType>
        static SIMDPPType<SIMDTraits<InType>::FastSize> convertFromFastType(typename SIMDTraits<InType>::FastType inReg)
        {
            simdpp::uint64<SIMDTraits<InType>::FastSize> outReg = simdpp::to_uint64(inReg);
            return outReg;
        }
    };

    // Depends on instruction set
    template <>
    constexpr bool SIMDTraits<unsigned long>::canConvertTo<float>() {return bool(SIMDPP_HAS_INT64_TO_FLOAT32_CONVERSION);}

    // Depends on instruction set
    template <>
    constexpr bool SIMDTraits<unsigned long>::canConvertTo<double>() {return bool(SIMDPP_HAS_INT64_TO_FLOAT64_CONVERSION);}

    template <>
    struct SIMDTraits<unsigned long long>
    {
        static constexpr size_t FastSize = SIMDPP_FAST_INT64_SIZE;

        template <size_t N>
        using SIMDPPType = simdpp::uint64<N>;

        using FastType = SIMDPPType<FastSize>;

        // Depends on instruction set
        static constexpr bool Supported = SIMDPP_HAS_INT64_SIMD;

        template <typename OutType>
        static constexpr bool canConvertTo() {return SIMDTraits<unsigned long>::canConvertTo<OutType>();}

        template <typename InType>
        static SIMDPPType<SIMDTraits<InType>::FastSize> convertFromFastType(typename SIMDTraits<InType>::FastType inReg)
        {
            simdpp::uint64<SIMDTraits<InType>::FastSize> outReg = simdpp::to_uint64(inReg);
            return outReg;
        }
    };

    template <>
    constexpr bool SIMDTraits<unsigned long long>::canConvertTo<float>() {return bool(SIMDPP_HAS_INT64_TO_FLOAT32_CONVERSION);}

    template <>
    constexpr bool SIMDTraits<unsigned long long>::canConvertTo<double>() {return bool(SIMDPP_HAS_INT64_TO_FLOAT64_CONVERSION);}

    template <>
    struct SIMDTraits<float>
    {
        static constexpr size_t FastSize = SIMDPP_FAST_FLOAT32_SIZE;

        template <size_t N>
        using SIMDPPType = simdpp::float32<N>;

        using FastType = SIMDPPType<FastSize>;

        // Depends on instruction set
        static constexpr bool Supported = SIMDPP_HAS_FLOAT32_SIMD;

        template <typename OutType>
        static constexpr bool canConvertTo() {return true;}

        template <typename InType>
        static SIMDPPType<SIMDTraits<InType>::FastSize> convertFromFastType(typename SIMDTraits<InType>::FastType inReg)
        {
            simdpp::float32<SIMDTraits<InType>::FastSize> outReg = simdpp::to_float32(inReg);
            return outReg;
        }
    };

    template <>
    constexpr bool SIMDTraits<float>::canConvertTo<long>() {return bool(SIMDPP_HAS_FLOAT32_TO_INT64_CONVERSION);}

    template <>
    constexpr bool SIMDTraits<float>::canConvertTo<long long>() {return bool(SIMDPP_HAS_FLOAT32_TO_INT64_CONVERSION);}

    template <>
    constexpr bool SIMDTraits<float>::canConvertTo<unsigned long>() {return bool(SIMDPP_HAS_FLOAT32_TO_UINT64_CONVERSION);}

    template <>
    constexpr bool SIMDTraits<float>::canConvertTo<unsigned long long>() {return bool(SIMDPP_HAS_FLOAT32_TO_UINT64_CONVERSION);}

    template <>
    struct SIMDTraits<double>
    {
        static constexpr size_t FastSize = SIMDPP_FAST_FLOAT64_SIZE;

        template <size_t N>
        using SIMDPPType = simdpp::float64<N>;

        using FastType = SIMDPPType<FastSize>;

        // Depends on instruction set
        static constexpr bool Supported = SIMDPP_HAS_FLOAT64_SIMD;

        template <typename OutType>
        static constexpr bool canConvertTo() {return true;}

        template <typename InType>
        static SIMDPPType<SIMDTraits<InType>::FastSize> convertFromFastType(typename SIMDTraits<InType>::FastType inReg)
        {
            simdpp::float64<SIMDTraits<InType>::FastSize> outReg = simdpp::to_float64(inReg);
            return outReg;
        }
    };

    template <>
    constexpr bool SIMDTraits<double>::canConvertTo<long>() {return bool(SIMDPP_HAS_FLOAT64_TO_INT64_CONVERSION);}

    template <>
    constexpr bool SIMDTraits<double>::canConvertTo<long long>() {return bool(SIMDPP_HAS_FLOAT64_TO_INT64_CONVERSION);}

    template <>
    constexpr bool SIMDTraits<double>::canConvertTo<std::uint32_t>() {return bool(SIMDPP_HAS_FLOAT64_TO_UINT32_CONVERSION);}

    template <>
    constexpr bool SIMDTraits<double>::canConvertTo<unsigned long>() {return bool(SIMDPP_HAS_FLOAT64_TO_UINT64_CONVERSION);}

    template <>
    constexpr bool SIMDTraits<double>::canConvertTo<unsigned long long>() {return bool(SIMDPP_HAS_FLOAT64_TO_UINT64_CONVERSION);}

    template <typename InType, typename OutType>
    struct BothComplex: std::integral_constant<bool,
        Pothos::Util::is_complex<InType>::value &&
        Pothos::Util::is_complex<OutType>::value
    > {};

    template <typename InType, typename OutType>
    struct NeitherComplex: std::integral_constant<bool,
        !Pothos::Util::is_complex<InType>::value &&
        !Pothos::Util::is_complex<OutType>::value
    > {};

    // See: libsimdpp/simdpp/capabilities.h
    template <typename InType, typename OutType>
    struct CanInstructionSetConvert: std::integral_constant<bool,
        NeitherComplex<InType, OutType>::value &&
        !std::is_same<InType, OutType>::value &&
        // For some reason, double -> integral conversions don't compile with some
        // architectures.
#if SIMDPP_ARCH_PP_NS_USE_POPCNT_INSN
        !(std::is_same<double, InType>::value && std::is_integral<OutType>::value) &&
#endif
        !(std::is_floating_point<InType>::value && std::is_integral<OutType>::value) &&
        SIMDTraits<InType>::Supported && SIMDTraits<OutType>::Supported &&
        SIMDTraits<InType>::template canConvertTo<OutType>()> {};

    template <typename InType, typename OutType, typename Ret>
    using EnableIfInstructionSetCanConvert = typename std::enable_if<
        CanInstructionSetConvert<InType, OutType>::value,
        Ret
    >::type;

    template <typename InType, typename OutType, typename Ret>
    using EnableIfNonComplexTypesMatch = typename std::enable_if<
        std::is_same<InType, OutType>::value &&
        !Pothos::Util::is_complex<InType>::value,
        Ret
    >::type;

    template <typename InType, typename OutType, typename Ret>
    using EnableIfNeitherComplex = typename std::enable_if<
        CanInstructionSetConvert<InType, OutType>::value && NeitherComplex<InType, OutType>::value,
        Ret
    >::type;

    template <typename InType, typename OutType, typename Ret>
    using EnableIfBothComplex = typename std::enable_if<BothComplex<InType, OutType>::value, Ret>::type;

    template <typename InType, typename OutType, typename Ret>
    using EnableIfConversionUnsupported = typename std::enable_if<
        !CanInstructionSetConvert<InType, OutType>::value && NeitherComplex<InType, OutType>::value && !std::is_same<InType, OutType>::value,
        Ret
    >::type;

    template <typename InType, typename OutType>
    static EnableIfInstructionSetCanConvert<InType, OutType, void>
    simdConvertBuffer(
        const InType* in,
        OutType* out,
        size_t bufferLen)
    {
        static constexpr size_t FrameSize = SIMDTraits<InType>::FastSize;
        using SIMDPPTypeIn = typename SIMDTraits<InType>::FastType;

        const auto numFrames = bufferLen / FrameSize;

        const InType* inPtr = in;
        OutType* outPtr = out;

        for(size_t frameIndex = 0; frameIndex < numFrames; ++frameIndex)
        {
            SIMDPPTypeIn inReg = simdpp::load_u(inPtr);
            auto outReg = SIMDTraits<OutType>::template convertFromFastType<InType>(inReg);
            simdpp::store_u(outPtr, outReg);

            inPtr += FrameSize;
            outPtr += FrameSize;
        }

        // Perform remaining conversions manually.
        for(size_t i = (FrameSize * numFrames); i < bufferLen; ++i)
        {
            out[i] = static_cast<OutType>(in[i]);
        }
    }

    template <typename InType, typename OutType>
    static EnableIfNonComplexTypesMatch<InType, OutType, void>
    simdConvertBuffer(
        const InType* in,
        OutType* out,
        size_t bufferLen)
    {
        std::memcpy(out, in, (bufferLen*sizeof(InType)));
    }

    template <typename InType, typename OutType>
    static EnableIfConversionUnsupported<InType, OutType, void>
    simdConvertBuffer(
        const InType* in,
        OutType* out,
        size_t bufferLen)
    {
        static_assert(NeitherComplex<InType, OutType>::value, "Unsupported overload called with complex types");

        for(size_t i = 0; i < bufferLen; ++i)
        {
            out[i] = static_cast<OutType>(in[i]);
        }
    }

    template <typename InType, typename OutType>
    static EnableIfBothComplex<InType, OutType, void>
    simdConvertBuffer(
        const InType* in,
        OutType* out,
        size_t bufferLen)
    {
        using ScalarInType = typename InType::value_type;
        using ScalarOutType = typename OutType::value_type;

        static_assert(NeitherComplex<ScalarInType, ScalarOutType>::value, "Complex overload called with double-complex types");

        simdConvertBuffer<ScalarInType, ScalarOutType>(
            (const ScalarInType*)in,
            (ScalarOutType*)out,
            (bufferLen*2));
    }
}

template <typename InType, typename OutType>
void simdConvertBuffer(const void* in, void* out, size_t bufferLen)
{
    detail::simdConvertBuffer<InType, OutType>(
        (const InType*) in,
        (OutType*)out,
        bufferLen);
}

}

// This generates the underlying code that queries the runnable instruction
// sets and chooses the most optimal to use. This code is only generated in
// the first of the files generated by SIMDPP's CMake module.
SIMDPP_MAKE_DISPATCHER(
    (template<typename InType, typename OutType>)
    (<InType, OutType>)
    (void)(simdConvertBuffer)
    ((const void*) in, (void*) out, (size_t) bufferLen))

// Separate dispatcher macros because it there are only
// underlying overloads for so many template specializations
// at once.
#define INSTANTIATE_DISPATCHERS(T) \
    SIMDPP_INSTANTIATE_DISPATCHER( \
        (template void simdConvertBuffer<T, char>(const void* in, void* out, size_t bufferLen)), \
        (template void simdConvertBuffer<T, std::int8_t>(const void* in, void* out, size_t bufferLen)), \
        (template void simdConvertBuffer<T, std::int16_t>(const void* in, void* out, size_t bufferLen)), \
        (template void simdConvertBuffer<T, std::int32_t>(const void* in, void* out, size_t bufferLen)), \
        (template void simdConvertBuffer<T, long>(const void* in, void* out, size_t bufferLen)), \
        (template void simdConvertBuffer<T, long long>(const void* in, void* out, size_t bufferLen)), \
        (template void simdConvertBuffer<T, std::uint8_t>(const void* in, void* out, size_t bufferLen)), \
        (template void simdConvertBuffer<T, std::uint16_t>(const void* in, void* out, size_t bufferLen)), \
        (template void simdConvertBuffer<T, std::uint32_t>(const void* in, void* out, size_t bufferLen)), \
        (template void simdConvertBuffer<T, unsigned long>(const void* in, void* out, size_t bufferLen)), \
        (template void simdConvertBuffer<T, unsigned long long>(const void* in, void* out, size_t bufferLen)), \
        (template void simdConvertBuffer<T, float>(const void* in, void* out, size_t bufferLen)), \
        (template void simdConvertBuffer<T, double>(const void* in, void* out, size_t bufferLen)) \
    ) \
    SIMDPP_INSTANTIATE_DISPATCHER( \
        (template void simdConvertBuffer<std::complex<T>, std::complex<char>>(const void* in, void* out, size_t bufferLen)), \
        (template void simdConvertBuffer<std::complex<T>, std::complex<std::int8_t>>(const void* in, void* out, size_t bufferLen)), \
        (template void simdConvertBuffer<std::complex<T>, std::complex<std::int16_t>>(const void* in, void* out, size_t bufferLen)), \
        (template void simdConvertBuffer<std::complex<T>, std::complex<std::int32_t>>(const void* in, void* out, size_t bufferLen)), \
        (template void simdConvertBuffer<std::complex<T>, std::complex<long>>(const void* in, void* out, size_t bufferLen)), \
        (template void simdConvertBuffer<std::complex<T>, std::complex<long long>>(const void* in, void* out, size_t bufferLen)), \
        (template void simdConvertBuffer<std::complex<T>, std::complex<std::uint8_t>>(const void* in, void* out, size_t bufferLen)), \
        (template void simdConvertBuffer<std::complex<T>, std::complex<std::uint16_t>>(const void* in, void* out, size_t bufferLen)), \
        (template void simdConvertBuffer<std::complex<T>, std::complex<std::uint32_t>>(const void* in, void* out, size_t bufferLen)), \
        (template void simdConvertBuffer<std::complex<T>, std::complex<unsigned long>>(const void* in, void* out, size_t bufferLen)), \
        (template void simdConvertBuffer<std::complex<T>, std::complex<unsigned long long>>(const void* in, void* out, size_t bufferLen)), \
        (template void simdConvertBuffer<std::complex<T>, std::complex<float>>(const void* in, void* out, size_t bufferLen)), \
        (template void simdConvertBuffer<std::complex<T>, std::complex<double>>(const void* in, void* out, size_t bufferLen)) \
    )

INSTANTIATE_DISPATCHERS(char)
INSTANTIATE_DISPATCHERS(std::int8_t)
INSTANTIATE_DISPATCHERS(std::int16_t)
INSTANTIATE_DISPATCHERS(std::int32_t)
INSTANTIATE_DISPATCHERS(long)
INSTANTIATE_DISPATCHERS(long long)
INSTANTIATE_DISPATCHERS(std::uint8_t)
INSTANTIATE_DISPATCHERS(std::uint16_t)
INSTANTIATE_DISPATCHERS(std::uint32_t)
INSTANTIATE_DISPATCHERS(unsigned long)
INSTANTIATE_DISPATCHERS(unsigned long long)
INSTANTIATE_DISPATCHERS(float)
INSTANTIATE_DISPATCHERS(double)
