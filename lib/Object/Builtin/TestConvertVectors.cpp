// Copyright (c) 2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Util/Templates.hpp>
#include <Pothos/Util/TypeInfo.hpp>

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
    (std::is_integral<InType>::value || std::is_integral<OutType>::value), InType>::type randType()
{
    return InType(rng.next(100));
}

template <typename InType, typename OutType>
typename std::enable_if<
    std::is_floating_point<InType>::value &&
    std::is_floating_point<OutType>::value, InType>::type randType()
{
    return InType(rng.nextFloat());
}

template <typename InType, typename OutType>
typename std::enable_if<
    Pothos::Util::is_complex<InType>::value &&
    !Pothos::Util::is_complex<OutType>::value, InType>::type randType()
{
    using ScalarInType = typename InType::value_type;

    return InType(randType<ScalarInType, OutType>(), randType<ScalarInType, OutType>());
}

template <typename InType, typename OutType>
typename std::enable_if<
    !Pothos::Util::is_complex<InType>::value &&
    Pothos::Util::is_complex<OutType>::value, InType>::type randType()
{
    using ScalarOutType = typename OutType::value_type;

    return randType<InType, ScalarOutType>();
}

template <typename InType, typename OutType>
typename std::enable_if<
    Pothos::Util::is_complex<InType>::value &&
    Pothos::Util::is_complex<OutType>::value, InType>::type randType()
{
    using ScalarInType = typename InType::value_type;
    using ScalarOutType = typename OutType::value_type;

    return InType(randType<ScalarInType, ScalarOutType>(), randType<ScalarInType, ScalarOutType>());
}

template <typename InType, typename OutType>
std::vector<InType> getInputVector(size_t numElems)
{
    std::vector<InType> inputVec(numElems);
    for(size_t i = 0; i < inputVec.size(); ++i)
    {
        inputVec[i] = randType<InType, OutType>();
    }

    return inputVec;
}

/***********************************************************************
 * templated conversion loop
 **********************************************************************/
template <typename InType, typename OutType>
void testVectorConvert()
{
    const size_t numElems = rng.next(1024);

    std::cout << "testVectorConvert: " << Pothos::Util::typeInfoToString(typeid(InType))
              << " to " << Pothos::Util::typeInfoToString(typeid(OutType))
              << "..." << std::flush;

    auto inputVec = getInputVector<InType, OutType>(numElems);
    auto outputVec = Pothos::Object(inputVec).convert<std::vector<OutType>>();

    for(size_t i = 0; i < numElems; ++i)
    {
        if(!checkEqual(inputVec[i], outputVec[i]))
        {
            std::cerr << "elem " << i << ": " << Pothos::Object(inputVec[i]).toString()
                      << " != " << Pothos::Object(outputVec[i]).toString() << std::endl;
            POTHOS_TEST_TRUE(checkEqual(inputVec[i], outputVec[i]));
        }
    }
    std::cout << "OK" << std::endl;
}

/***********************************************************************
 * templated test dispatch
 **********************************************************************/
template <typename InType>
void dispatchTestsForType(void)
{
    testVectorConvert<InType, char>();

    testVectorConvert<InType, std::int8_t>();
    testVectorConvert<InType, std::int16_t>();
    testVectorConvert<InType, std::int32_t>();
    testVectorConvert<InType, long>();
    testVectorConvert<InType, long long>();

    testVectorConvert<InType, std::uint8_t>();
    testVectorConvert<InType, std::uint16_t>();
    testVectorConvert<InType, std::uint32_t>();
    testVectorConvert<InType, unsigned long>();
    testVectorConvert<InType, unsigned long long>();

    testVectorConvert<InType, float>();
    testVectorConvert<InType, double>();
}

}

/***********************************************************************
 * conversion test cases
 **********************************************************************/
POTHOS_TEST_BLOCK("/object/tests", test_convert_vectors)
{
    dispatchTestsForType<char>();
    dispatchTestsForType<std::int8_t>();
    dispatchTestsForType<std::int16_t>();
    dispatchTestsForType<std::int32_t>();
    dispatchTestsForType<long>();
    dispatchTestsForType<long long>();
    dispatchTestsForType<std::uint8_t>();
    dispatchTestsForType<std::uint16_t>();
    dispatchTestsForType<std::uint32_t>();
    dispatchTestsForType<unsigned long>();
    dispatchTestsForType<unsigned long long>();
    dispatchTestsForType<float>();
    dispatchTestsForType<double>();
}
