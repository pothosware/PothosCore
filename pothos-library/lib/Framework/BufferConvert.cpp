// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/BufferChunk.hpp>
#include <Pothos/Framework/Exception.hpp>
#include <Poco/SingletonHolder.h>
#include <functional>
#include <complex>
#include <map>

/***********************************************************************
 * templated conversions
 **********************************************************************/
template <typename InType, typename OutType>
void rawConvert(const InType *in, OutType *out, size_t num)
{
    for (size_t i = 0; i < num; i++) out[i] = OutType(in[i]);
}

template <typename InType, typename OutType>
void rawConvert(const InType *in, std::complex<OutType> *out, size_t num)
{
    for (size_t i = 0; i < num; i++) out[i] = std::complex<OutType>(OutType(in[i]));
}

template <typename InType, typename OutType>
void rawConvert(const std::complex<InType> *in, std::complex<OutType> *out, size_t num)
{
    for (size_t i = 0; i < num; i++) out[i] = std::complex<OutType>(OutType(in[i].real()), OutType(in[i].imag()));
}

template <typename InType, typename OutType>
void rawConvert(const std::complex<InType> *in, OutType *outRe, OutType *outIm, size_t num)
{
    for (size_t i = 0; i < num; i++)
    {
        outRe[i] = OutType(in[i].real());
        outIm[i] = OutType(in[i].imag());
    }
}

/***********************************************************************
 * bound conversions
 **********************************************************************/
struct BufferConvertImpl
{
    BufferConvertImpl(void)
    {
        
    }

    std::map<size_t, std::function<void(const void *, void *, const size_t)>> convertMap;
    std::map<size_t, std::function<void(const void *, void *, void *, const size_t)>> convertComplexMap;
};

static BufferConvertImpl &getBufferConvertImpl(void)
{
    static Poco::SingletonHolder<BufferConvertImpl> sh;
    return *sh.get();
}

/***********************************************************************
 * conversion implementation
 **********************************************************************/
Pothos::BufferChunk Pothos::BufferChunk::convert(const DType &outDType, const size_t numElems)
{
    const auto primElems = (numElems*this->dtype.size())/this->dtype.elemSize();
    const auto outElems = primElems*outDType.size()/outDType.elemSize();

    auto it = getBufferConvertImpl().convertMap.find(dtype.elemType());
    if (it == getBufferConvertImpl().convertMap.end()) throw Pothos::BufferConvertError(
        "Pothos::BufferChunk::convert("+dtype.toString()+")", "cant convert from " + this->dtype.toString());
    Pothos::BufferChunk out(outDType, outElems);

    it->second(this->as<const void *>(), out.as<void *>(), primElems);
    return out;
}

std::pair<Pothos::BufferChunk, Pothos::BufferChunk> Pothos::BufferChunk::convertComplex(const DType &outDType, const size_t numElems)
{
    const auto primElems = (numElems*this->dtype.size())/this->dtype.elemSize();
    const auto outElems = primElems*outDType.size()/outDType.elemSize();

    auto it = getBufferConvertImpl().convertComplexMap.find(dtype.elemType());
    if (it == getBufferConvertImpl().convertComplexMap.end()) throw Pothos::BufferConvertError(
        "Pothos::BufferChunk::convertComplex("+dtype.toString()+")", "cant convert from " + this->dtype.toString());
    Pothos::BufferChunk outRe(outDType, outElems);
    Pothos::BufferChunk outIm(outDType, outElems);

    it->second(this->as<const void *>(), outRe.as<void *>(), outIm.as<void *>(), primElems);
    return std::make_pair(outRe, outIm);
}
