// Copyright (c) 2014-2016 Josh Blum
//                    2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Util/Templates.hpp>
#include <Pothos/Util/TypeInfo.hpp>
#include <Poco/Format.h>
#include <Poco/Random.h>
#include <random>
#include <cstdint>
#include <complex>
#include <iostream>
#include <type_traits>

namespace
{

static Poco::Random rng;

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
 * Templated random numbers
 *
 * Use both types to determine the random number so we can properly
 * test float <-> double and floatXX <-> intXX.
 **********************************************************************/
template <typename InType, typename OutType>
typename std::enable_if<
    !Pothos::Util::is_complex<InType>::value &&
    !Pothos::Util::is_complex<OutType>::value &&
    (std::is_integral<InType>::value || std::is_integral<OutType>::value), void>::type
randType(InType &val)
{
    val = InType(rng.next(100));
}

template <typename InType, typename OutType>
typename std::enable_if<
    std::is_floating_point<InType>::value &&
    std::is_floating_point<OutType>::value, void>::type
randType(InType &val)
{
    val = InType(rng.nextFloat());
}

template <typename InType, typename OutType>
typename std::enable_if<
    Pothos::Util::is_complex<InType>::value &&
    !Pothos::Util::is_complex<OutType>::value, void>::type
randType(InType &val)
{
    using ScalarInType = typename InType::value_type;

    ScalarInType real, imag;
    randType<ScalarInType, OutType>(real);
    randType<ScalarInType, OutType>(imag);

    val = InType(real, imag);
}

template <typename InType, typename OutType>
typename std::enable_if<
    !Pothos::Util::is_complex<InType>::value &&
    Pothos::Util::is_complex<OutType>::value, void>::type
randType(InType &val)
{
    using ScalarOutType = typename OutType::value_type;
    randType<InType, ScalarOutType>(val);
}

template <typename InType, typename OutType>
typename std::enable_if<
    Pothos::Util::is_complex<InType>::value &&
    Pothos::Util::is_complex<OutType>::value, void>::type
randType(InType &val)
{
    using ScalarInType = typename InType::value_type;
    using ScalarOutType = typename OutType::value_type;

    ScalarInType real, imag;
    randType<ScalarInType, ScalarOutType>(real);
    randType<ScalarInType, ScalarOutType>(imag);

    val = InType(real, imag);
}

/***********************************************************************
 * templated conversion loop
 **********************************************************************/
template <typename InType, typename OutType>
void testBufferConvert(const size_t inVlen, const size_t outVlen)
{
    const size_t numElems = rng.next(1024);
    Pothos::BufferChunk b0(Pothos::DType(typeid(InType), inVlen), numElems);
    const auto primElems = b0.length/b0.dtype.elemSize();

    //random fill primitive elements
    for (size_t i = 0; i < primElems; i++) randType<InType, OutType>(b0.as<InType *>()[i]);

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
            std::cerr << "elem " << i << ": " << Pothos::Object(in).toString() << " != " << Pothos::Object(out).toString() << std::endl;
            POTHOS_TEST_TRUE(checkEqual(in, out));
        }
    }
    std::cout << "OK" << std::endl;
}

template <typename InType, typename OutType>
void testBufferConvertComplexComponents(const size_t inVlen, const size_t outVlen)
{
    const size_t numElems = rng.next(1024);
    Pothos::BufferChunk b0(Pothos::DType(typeid(InType), inVlen), numElems);
    const auto primElems = b0.length/b0.dtype.elemSize();

    //random fill primitive elements
    for (size_t i = 0; i < primElems; i++) randType<InType, std::complex<OutType>>(b0.as<InType *>()[i]);

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
            std::cerr << "elem " << i << ": " << Pothos::Object(in).toString() << " != "
                      << Pothos::Object(outRe).toString() << ", " << Pothos::Object(outIm).toString() << std::endl;
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

template <typename InType>
void dispatchTestsForType(void)
{
    dispatchTests<InType, std::int8_t>();
    dispatchTests<InType, std::int16_t>();
    dispatchTests<InType, std::int32_t>();
    dispatchTests<InType, std::int64_t>();

    dispatchTests<InType, std::uint8_t>();
    dispatchTests<InType, std::uint16_t>();
    dispatchTests<InType, std::uint32_t>();
    dispatchTests<InType, std::uint64_t>();

    dispatchTests<InType, float>();
    dispatchTests<InType, double>();
}

}

/***********************************************************************
 * conversion test cases
 **********************************************************************/
POTHOS_TEST_BLOCK("/framework/tests", test_buffer_convert)
{
    dispatchTestsForType<std::int8_t>();
    dispatchTestsForType<std::int16_t>();
    dispatchTestsForType<std::int32_t>();
    dispatchTestsForType<std::int64_t>();
    dispatchTestsForType<std::uint8_t>();
    dispatchTestsForType<std::uint16_t>();
    dispatchTestsForType<std::uint32_t>();
    dispatchTestsForType<std::uint64_t>();
    dispatchTestsForType<float>();
    dispatchTestsForType<double>();
}
