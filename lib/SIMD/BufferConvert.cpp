// Copyright (c) 2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include "BufferConversions.hpp"

#include <Pothos/Plugin.hpp>

#include <xsimd/xsimd.hpp>

#include <complex>
#include <type_traits>

#if !defined POTHOS_SIMD_NAMESPACE
#error Must define POTHOS_SIMD_NAMESPACE to build this file
#endif

#define _str(s) #s
#define str(s) _str(s)

namespace PothosSIMD { namespace POTHOS_SIMD_NAMESPACE {

    namespace detail
    {
        template <typename T>
        struct IsComplex : std::false_type {};

        template <typename T>
        struct IsComplex<std::complex<T>> : std::true_type {};

        template <typename InType, typename OutType>
        using EnableIfNeitherComplex = typename std::enable_if<!IsComplex<InType>::value && !IsComplex<OutType>::value>::type;

        template <typename InType, typename OutType>
        using EnableIfBothComplex = typename std::enable_if<IsComplex<InType>::value && IsComplex<OutType>::value>::type;

        template <typename InType, typename OutType>
        static EnableIfNeitherComplex<InType, OutType> bufferConvert(const InType* in, OutType* out, size_t len)
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
        static EnableIfBothComplex<InType, OutType> bufferConvert(const InType* in, OutType* out, size_t len)
        {
            using ScalarInType = typename InType::value_type;
            using ScalarOutType = typename OutType::value_type;

            bufferConvert<ScalarInType, ScalarOutType>((const ScalarInType*)in, (ScalarOutType*)out, (len * 2));
        }
    }

    template <typename InType, typename OutType>
    void bufferConvert(const void* in, void* out, size_t len)
    {
        detail::bufferConvert<InType, OutType>((const InType*)in, (OutType*)out, len);
    }

#define DECLARE_BUFFERCONVERT_FUNCS(InType) \
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

    DECLARE_BUFFERCONVERT_FUNCS(std::int8_t)
    DECLARE_BUFFERCONVERT_FUNCS(std::int16_t)
    DECLARE_BUFFERCONVERT_FUNCS(std::int32_t)
    DECLARE_BUFFERCONVERT_FUNCS(std::int64_t)
    DECLARE_BUFFERCONVERT_FUNCS(std::uint8_t)
    DECLARE_BUFFERCONVERT_FUNCS(std::uint16_t)
    DECLARE_BUFFERCONVERT_FUNCS(std::uint32_t)
    DECLARE_BUFFERCONVERT_FUNCS(std::uint64_t)
    DECLARE_BUFFERCONVERT_FUNCS(float)
    DECLARE_BUFFERCONVERT_FUNCS(double)

    template <typename InType, typename OutType>
    static void addBufferConverterToMap(BufferConvertFcnMap* pFcnMap)
    {
        assert(pFcnMap);

        auto inDType = Pothos::DType(typeid(InType));
        auto inComplexDType = Pothos::DType(typeid(std::complex<InType>));
        auto outDType = Pothos::DType(typeid(OutType));
        auto outComplexDType = Pothos::DType(typeid(std::complex<OutType>));

        auto hash = dtypeIOToHash(inDType, outDType);
        auto complexHash = dtypeIOToHash(inComplexDType, outComplexDType);

        pFcnMap->emplace(hash, &bufferConvert<InType, OutType>);
        pFcnMap->emplace(complexHash, &bufferConvert<std::complex<InType>, std::complex<OutType>>);
    }

    template <typename InType>
    static void addBufferConverterToMap(BufferConvertFcnMap* pFcnMap)
    {
        addBufferConverterToMap<InType, std::int8_t>(pFcnMap);
        addBufferConverterToMap<InType, std::int16_t>(pFcnMap);
        addBufferConverterToMap<InType, std::int32_t>(pFcnMap);
        addBufferConverterToMap<InType, std::int64_t>(pFcnMap);
        addBufferConverterToMap<InType, std::uint8_t>(pFcnMap);
        addBufferConverterToMap<InType, std::uint16_t>(pFcnMap);
        addBufferConverterToMap<InType, std::uint32_t>(pFcnMap);
        addBufferConverterToMap<InType, std::uint64_t>(pFcnMap);
        addBufferConverterToMap<InType, float>(pFcnMap);
        addBufferConverterToMap<InType, double>(pFcnMap);
    }

    // Note: putting the static block inside the arch-specific namespace
    // will remove symbol collision.
    pothos_static_block(registerPothosSIMDConverters)
    {
        BufferConvertFcnMap fcnMap;

        addBufferConverterToMap<std::int8_t>(&fcnMap);
        addBufferConverterToMap<std::int16_t>(&fcnMap);
        addBufferConverterToMap<std::int32_t>(&fcnMap);
        addBufferConverterToMap<std::int64_t>(&fcnMap);
        addBufferConverterToMap<std::uint8_t>(&fcnMap);
        addBufferConverterToMap<std::uint16_t>(&fcnMap);
        addBufferConverterToMap<std::uint32_t>(&fcnMap);
        addBufferConverterToMap<std::uint64_t>(&fcnMap);
        addBufferConverterToMap<float>(&fcnMap);
        addBufferConverterToMap<double>(&fcnMap);

        const auto pluginPath = std::string("/simd/buffer_converter_maps/") + str(POTHOS_SIMD_NAMESPACE);

        Pothos::PluginRegistry::add(pluginPath, std::move(fcnMap));
    }
}}