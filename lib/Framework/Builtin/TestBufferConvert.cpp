// Copyright (c) 2014-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Poco/Format.h>
#include <random>
#include <cstdint>
#include <complex>
#include <iostream>

/***********************************************************************
 * templated equality check
 **********************************************************************/
template <typename InType, typename OutType>
bool checkEqual(const InType in, const OutType out)
{
    return double(in) == double(out);
}

template <typename InType, typename OutType>
bool checkEqual(const InType in, const std::complex<OutType> out)
{
    return checkEqual(in, out.real()) and checkEqual(InType(0), out.imag());
}

template <typename InType, typename OutType>
bool checkEqual(const std::complex<InType> in, const std::complex<OutType> out)
{
    return checkEqual(in.real(), out.real()) and checkEqual(in.imag(), out.imag());
}

template <typename InType, typename OutType>
bool checkEqual(const std::complex<InType> in, const OutType outRe, const OutType outIm)
{
    return checkEqual(in.real(), outRe) and checkEqual(in.imag(), outIm);
}

/***********************************************************************
 * templated random numbers
 **********************************************************************/
template <typename Type>
void randType(Type &val)
{
    val = Type(std::rand());
}

template <typename Type>
void randType(std::complex<Type> &val)
{
    val = std::complex<Type>(Type(std::rand()), Type(std::rand()));
}

/***********************************************************************
 * templated conversion loop
 **********************************************************************/
template <typename InType, typename OutType>
void testBufferConvert(const size_t inVlen, const size_t outVlen)
{
    const size_t numElems = 500 + (std::rand() % 100);
    Pothos::BufferChunk b0(Pothos::DType(typeid(InType), inVlen), numElems);
    const auto primElems = b0.length/b0.dtype.elemSize();

    //random fill primitive elements
    for (size_t i = 0; i < primElems; i++) randType(b0.as<InType *>()[i]);

    //convert
    const auto b1 = b0.convert(Pothos::DType(typeid(OutType), outVlen), numElems);

    //check
    std::cout << "testBufferConvert: " << b0.dtype.toString() << " to " << b1.dtype.toString() << "...\t" << std::flush;
    for (size_t i = 0; i < primElems; i++)
    {
        const auto in = b0.as<const InType *>()[i];
        const auto out = b1.as<const OutType *>()[i];
        if (not checkEqual(in, out))
        {
            std::cerr << "elem " << i << ": " << in << " != " << out << std::endl;
            POTHOS_TEST_TRUE(checkEqual(in, out));
        }
    }
    std::cout << "OK" << std::endl;
}

template <typename InType, typename OutType>
void testBufferConvertComplexComponents(const size_t inVlen, const size_t outVlen)
{
    const size_t numElems = 500 + (std::rand() % 100);
    Pothos::BufferChunk b0(Pothos::DType(typeid(InType), inVlen), numElems);
    const auto primElems = b0.length/b0.dtype.elemSize();

    //random fill primitive elements
    for (size_t i = 0; i < primElems; i++) randType(b0.as<InType *>()[i]);

    //convert
    const auto b1 = b0.convertComplex(Pothos::DType(typeid(OutType), outVlen), numElems);

    //check
    std::cout << "testBufferConvertComplexComponents: " << b0.dtype.toString() << " to " << b1.first.dtype.toString() << "...\t" << std::flush;
    for (size_t i = 0; i < primElems; i++)
    {
        const auto in = b0.as<const InType *>()[i];
        const auto outRe = b1.first.as<const OutType *>()[i];
        const auto outIm = b1.second.as<const OutType *>()[i];
        if (not checkEqual(in.real(), outRe) or not checkEqual(in.imag(), outIm))
        {
            std::cerr << "elem " << i << ": " << in << " != " << outRe << ", " << outIm << std::endl;
            POTHOS_TEST_TRUE(checkEqual(in.real(), outRe) and checkEqual(in.imag(), outIm));
        }
    }
    std::cout << "OK" << std::endl;
}

/***********************************************************************
 * templated test dispatch
 **********************************************************************/
template <typename InType, typename OutType>
void dispatchTests(void)
{
    //real to real -- vary dtype
    testBufferConvert<InType, OutType>(1, 1);
    testBufferConvert<InType, OutType>(2, 1);
    testBufferConvert<InType, OutType>(1, 2);

    //real to complex -- vary dtype
    testBufferConvert<InType, std::complex<OutType>>(1, 1);
    testBufferConvert<InType, std::complex<OutType>>(2, 1);
    testBufferConvert<InType, std::complex<OutType>>(1, 2);

    //complex to complex -- vary dtype
    testBufferConvert<std::complex<InType>, std::complex<OutType>>(1, 1);
    testBufferConvert<std::complex<InType>, std::complex<OutType>>(2, 1);
    testBufferConvert<std::complex<InType>, std::complex<OutType>>(1, 2);

    //complex to components -- vary dtype
    testBufferConvertComplexComponents<std::complex<InType>, OutType>(1, 1);
    testBufferConvertComplexComponents<std::complex<InType>, OutType>(2, 1);
    testBufferConvertComplexComponents<std::complex<InType>, OutType>(1, 2);
}

/***********************************************************************
 * conversion test cases
 **********************************************************************/
POTHOS_TEST_BLOCK("/framework/tests", test_buffer_convert)
{
    //same types
    dispatchTests<int, int>();
    dispatchTests<long long, long long>();
    dispatchTests<double, double>();

    //double <-> integer
    dispatchTests<double, int>();
    dispatchTests<int, double>();

    //switch signedness
    dispatchTests<signed int, unsigned int>();
    dispatchTests<unsigned int, signed int>();
    dispatchTests<unsigned int, double>();
    dispatchTests<double, unsigned int>();
    dispatchTests<long long, unsigned int>();
    dispatchTests<unsigned int, long long>();
}
