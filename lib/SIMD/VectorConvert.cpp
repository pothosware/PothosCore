// Copyright (c) 2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include <xsimd/xsimd.hpp>

#include <complex>
#include <vector>

#if !defined POTHOS_SIMD_NAMESPACE
#error Must define POTHOS_SIMD_NAMESPACE to build this file
#endif

namespace PothosSIMD { namespace POTHOS_SIMD_NAMESPACE {

    // See SIMD/BufferConvert.cpp
    template <typename InType, typename OutType>
    void bufferConvert(const void* in, void* out, size_t len);

    template <typename InType, typename OutType>
    std::vector<OutType> vectorConvert(const std::vector<InType>& in)
    {
        std::vector<OutType> out(in.size());
        bufferConvert<InType, OutType>(in.data(), out.data(), in.size());

        return out;
    }

    // Pothos enforces numeric bounds for std::vector conversions, so only add SIMD conversions
    // when we know the output type can contain all values of the input type.
#define DECLARE_VECTORCONVERT_FUNC(InType, OutType) \
    template std::vector<OutType> vectorConvert<InType, OutType>(const std::vector<InType>&); \
    template std::vector<std::complex<OutType>> vectorConvert<std::complex<InType>, std::complex<OutType>>(const std::vector<std::complex<InType>>&);

    DECLARE_VECTORCONVERT_FUNC(std::int8_t, std::int8_t)
    DECLARE_VECTORCONVERT_FUNC(std::int8_t, std::int16_t)
    DECLARE_VECTORCONVERT_FUNC(std::int8_t, std::int32_t)
    DECLARE_VECTORCONVERT_FUNC(std::int8_t, std::int64_t)
    DECLARE_VECTORCONVERT_FUNC(std::int8_t, float)
    DECLARE_VECTORCONVERT_FUNC(std::int8_t, double)

    DECLARE_VECTORCONVERT_FUNC(std::int16_t, std::int16_t)
    DECLARE_VECTORCONVERT_FUNC(std::int16_t, std::int32_t)
    DECLARE_VECTORCONVERT_FUNC(std::int16_t, std::int64_t)
    DECLARE_VECTORCONVERT_FUNC(std::int16_t, float)
    DECLARE_VECTORCONVERT_FUNC(std::int16_t, double)

    DECLARE_VECTORCONVERT_FUNC(std::int32_t, std::int32_t)
    DECLARE_VECTORCONVERT_FUNC(std::int32_t, std::int64_t)
    DECLARE_VECTORCONVERT_FUNC(std::int32_t, float)
    DECLARE_VECTORCONVERT_FUNC(std::int32_t, double)

    DECLARE_VECTORCONVERT_FUNC(std::int64_t, std::int64_t)
    DECLARE_VECTORCONVERT_FUNC(std::int64_t, float)
    DECLARE_VECTORCONVERT_FUNC(std::int64_t, double)

    DECLARE_VECTORCONVERT_FUNC(std::uint8_t, std::uint8_t)
    DECLARE_VECTORCONVERT_FUNC(std::uint8_t, std::uint16_t)
    DECLARE_VECTORCONVERT_FUNC(std::uint8_t, std::uint32_t)
    DECLARE_VECTORCONVERT_FUNC(std::uint8_t, std::uint64_t)
    DECLARE_VECTORCONVERT_FUNC(std::uint8_t, std::int16_t)
    DECLARE_VECTORCONVERT_FUNC(std::uint8_t, std::int32_t)
    DECLARE_VECTORCONVERT_FUNC(std::uint8_t, std::int64_t)
    DECLARE_VECTORCONVERT_FUNC(std::uint8_t, float)
    DECLARE_VECTORCONVERT_FUNC(std::uint8_t, double)

    DECLARE_VECTORCONVERT_FUNC(std::uint16_t, std::uint16_t)
    DECLARE_VECTORCONVERT_FUNC(std::uint16_t, std::uint32_t)
    DECLARE_VECTORCONVERT_FUNC(std::uint16_t, std::uint64_t)
    DECLARE_VECTORCONVERT_FUNC(std::uint16_t, std::int32_t)
    DECLARE_VECTORCONVERT_FUNC(std::uint16_t, std::int64_t)
    DECLARE_VECTORCONVERT_FUNC(std::uint16_t, float)
    DECLARE_VECTORCONVERT_FUNC(std::uint16_t, double)

    DECLARE_VECTORCONVERT_FUNC(std::uint32_t, std::uint32_t)
    DECLARE_VECTORCONVERT_FUNC(std::uint32_t, std::uint64_t)
    DECLARE_VECTORCONVERT_FUNC(std::uint32_t, std::int64_t)
    DECLARE_VECTORCONVERT_FUNC(std::uint32_t, float)
    DECLARE_VECTORCONVERT_FUNC(std::uint32_t, double)

    DECLARE_VECTORCONVERT_FUNC(std::uint64_t, std::uint64_t)
    DECLARE_VECTORCONVERT_FUNC(std::uint64_t, float)
    DECLARE_VECTORCONVERT_FUNC(std::uint64_t, double)

    DECLARE_VECTORCONVERT_FUNC(float, float)
    DECLARE_VECTORCONVERT_FUNC(float, double)

    DECLARE_VECTORCONVERT_FUNC(double, double)

}}