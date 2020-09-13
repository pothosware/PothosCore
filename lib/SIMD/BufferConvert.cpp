// Copyright (c) 2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include <xsimd/xsimd.hpp>

#include <complex>
#include <type_traits>

#if !defined POTHOS_SIMD_NAMESPACE
#error Must define POTHOS_SIMD_NAMESPACE to build this file
#endif

namespace PothosSIMD { namespace POTHOS_SIMD_NAMESPACE {

    template <typename T>
    struct IsComplex : std::false_type {};

    template <typename T>
    struct IsComplex<std::complex<T>> : std::true_type {};

    template <typename InType, typename OutType>
    using EnableIfNeitherComplex = typename std::enable_if<!IsComplex<InType>::value && !IsComplex<OutType>::value>::type;

    template <typename InType, typename OutType>
    using EnableIfBothComplex = typename std::enable_if<IsComplex<InType>::value && IsComplex<OutType>::value>::type;

    template <typename InType, typename OutType>
    static EnableIfNeitherComplex<InType, OutType> _bufferConvert(const InType* in, OutType* out, size_t len)
    {
        // Use input size, XSIMD will deal with the output batch sizes internally.
        static constexpr size_t simdSize = xsimd::simd_traits<InType>::size;
        const auto numSIMDFrames = len / simdSize;

        const InType* inPtr = in;
        OutType* outPtr = out;

        for (size_t frameIndex = 0; frameIndex < numSIMDFrames; ++frameIndex)
        {
            auto reg = xsimd::load_unaligned(inPtr);
            reg.store_unaligned(outPtr);

            inPtr += simdSize;
            outPtr += simdSize;
        }

        // Perform remaining operations manually.
        for (size_t elem = (simdSize * numSIMDFrames); elem < len; ++elem)
        {
            out[elem] = static_cast<OutType>(in[elem]);
        }
    }

    template <typename InType, typename OutType>
    static EnableIfBothComplex<InType, OutType> _bufferConvert(const InType* in, OutType* out, size_t len)
    {
        using ScalarInType = typename InType::value_type;
        using ScalarOutType = typename OutType::value_type;

        bufferConvert<ScalarInType, ScalarOutType>((const ScalarInType*)in, (ScalarOutType*)out, (len * 2));
    }

    template <typename InType, typename OutType>
    void bufferConvert(const void* in, void* out, size_t len)
    {
        _bufferConvert<InType, OutType>((const InType*)in, (OutType*)out, len);
    }

#define DECLARE_FUNCS(InType) \
    template void bufferConvert<InType, std::int8_t>(const void*, void*, size_t); \
    template void bufferConvert<InType, std::int16_t>(const void*, void*, size_t); \
    template void bufferConvert<InType, std::int32_t>(const void*, void*, size_t); \
    template void bufferConvert<InType, std::int64_t>(const void*, void*, size_t); \
    template void bufferConvert<InType, std::uint8_t>(const void*, void*, size_t); \
    template void bufferConvert<InType, std::uint16_t>(const void*, void*, size_t); \
    template void bufferConvert<InType, std::uint32_t>(const void*, void*, size_t); \
    template void bufferConvert<InType, std::uint64_t>(const void*, void*, size_t); \
    template void bufferConvert<InType, float>(const void*, void*, size_t); \
    template void bufferConvert<InType, double>(const void*, void*, size_t); \
    template void bufferConvert<std::complex<InType>, std::complex<std::int8_t>>(const void*, void*, size_t); \
    template void bufferConvert<std::complex<InType>, std::complex<std::int16_t>>(const void*, void*, size_t); \
    template void bufferConvert<std::complex<InType>, std::complex<std::int32_t>>(const void*, void*, size_t); \
    template void bufferConvert<std::complex<InType>, std::complex<std::int64_t>>(const void*, void*, size_t); \
    template void bufferConvert<std::complex<InType>, std::complex<std::uint8_t>>(const void*, void*, size_t); \
    template void bufferConvert<std::complex<InType>, std::complex<std::uint16_t>>(const void*, void*, size_t); \
    template void bufferConvert<std::complex<InType>, std::complex<std::uint32_t>>(const void*, void*, size_t); \
    template void bufferConvert<std::complex<InType>, std::complex<std::uint64_t>>(const void*, void*, size_t); \
    template void bufferConvert<std::complex<InType>, std::complex<float>>(const void*, void*, size_t); \
    template void bufferConvert<std::complex<InType>, std::complex<double>>(const void*, void*, size_t); \

    DECLARE_FUNCS(std::int8_t)
    DECLARE_FUNCS(std::int16_t)
    DECLARE_FUNCS(std::int32_t)
    DECLARE_FUNCS(std::int64_t)
    DECLARE_FUNCS(std::uint8_t)
    DECLARE_FUNCS(std::uint16_t)
    DECLARE_FUNCS(std::uint32_t)
    DECLARE_FUNCS(std::uint64_t)
    DECLARE_FUNCS(float)
    DECLARE_FUNCS(double)

}}