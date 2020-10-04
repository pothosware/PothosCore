// Copyright (c) 2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#pragma once

#include <Pothos/Framework.hpp>

#include <map>

namespace PothosSIMD
{
    using BufferConvertFcn = void(*)(const void*, void*, const size_t);
    using BufferConvertFcnMap = std::map<int, BufferConvertFcn>;

    inline int dtypeIOToHash(const Pothos::DType& in, const Pothos::DType& out)
    {
        return in.elemType() | (out.elemType() << 16); //safe assumption, elem type uses only first few bits
    }
    
    BufferConvertFcn bufferConvertDispatch(const Pothos::DType& inType, const Pothos::DType& outType);

    template <typename InType, typename OutType>
    inline BufferConvertFcn bufferConvertDispatch()
    {
        return bufferConvertDispatch(typeid(InType), typeid(OutType));
    }
}