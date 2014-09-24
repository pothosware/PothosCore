// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Object.hpp>
#include <Pothos/Framework/DType.hpp>
#include <Pothos/Framework/Exception.hpp>
#include <iostream>
/*
POTHOS_TEST_BLOCK("/framework/tests", test_dtype)
{
    Pothos::DType nullType;
    POTHOS_TEST_EQUAL(nullType.name(), std::string());
    POTHOS_TEST_EQUAL(nullType.shape().size(), 0);
    POTHOS_TEST_EQUAL(nullType.size(), 0);
    POTHOS_TEST_TRUE(nullType == Pothos::DType());

    Pothos::DType intType("int");
    std::cout << intType.toString() << std::endl;
    POTHOS_TEST_EQUAL(intType.size(), sizeof(int));

    Pothos::DType anotherIntType("signed int");
    std::cout << anotherIntType.toString() << std::endl;
    POTHOS_TEST_TRUE(anotherIntType == intType);

    Pothos::DType anotherOtherIntType(typeid(int));
    std::cout << anotherOtherIntType.toString() << std::endl;
    POTHOS_TEST_TRUE(anotherOtherIntType == intType);

    Pothos::Object justAnotherStringInt("int");
    POTHOS_TEST_TRUE(justAnotherStringInt.convert<Pothos::DType>() == intType);

    Pothos::DType::Shape vectorShape;
    vectorShape.push_back(10);
    Pothos::DType vectorType("float", vectorShape);
    std::cout << vectorType.toString() << std::endl;
    POTHOS_TEST_EQUAL(vectorType.size(), sizeof(float)*vectorShape[0]);

    Pothos::DType::Shape matrixShape;
    matrixShape.push_back(12);
    matrixShape.push_back(13);
    Pothos::DType matrixType("double", matrixShape);
    std::cout << matrixType.toString() << std::endl;
    POTHOS_TEST_EQUAL(matrixType.size(), sizeof(double)*matrixShape[0]*matrixShape[1]);

    Pothos::DType matrixTypeAgain("double, 12, 13");
    std::cout << matrixTypeAgain.toString() << std::endl;
    POTHOS_TEST_TRUE(matrixTypeAgain == matrixType);
    POTHOS_TEST_THROWS(Pothos::DType("double, foo"), Pothos::DTypeUnknownError);
    POTHOS_TEST_THROWS(Pothos::DType("double, 2 3"), Pothos::DTypeUnknownError);
    POTHOS_TEST_THROWS(Pothos::DType("double, -10"), Pothos::DTypeUnknownError);

    POTHOS_TEST_THROWS(Pothos::DType("madeUpUnknownType"), Pothos::DTypeUnknownError);
    Pothos::DType madeUpUnknownType("madeUpUnknownType", 64); //OK when element size specified
    POTHOS_TEST_EQUAL(madeUpUnknownType.size(), 64);
    POTHOS_TEST_EQUAL(madeUpUnknownType.shape().size(), 0);
    POTHOS_TEST_EQUAL(madeUpUnknownType.name(), "madeUpUnknownType");
}

*/

