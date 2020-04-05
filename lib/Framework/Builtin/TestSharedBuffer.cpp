// Copyright (c) 2013-2014 Josh Blum
//                    2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework/SharedBuffer.hpp>
#include <Pothos/Framework/Exception.hpp>
#include <cstdlib> //rand

POTHOS_TEST_BLOCK("/framework/tests", test_generic_shared_buffer)
{
    auto b0 = Pothos::SharedBuffer::make(1024);
    POTHOS_TEST_NOT_EQUAL(b0.getAddress(), 0);
    POTHOS_TEST_TRUE((b0.getAddress() & 0xf) == 0); //has alignment
    POTHOS_TEST_EQUAL(b0.getLength(), 1024);
    for (size_t i = 0; i < b0.getLength()/sizeof(int); i++)
    {
        int *p = reinterpret_cast<int *>(b0.getAddress());
        const int randNum = std::rand();
        p[i] = randNum;
        POTHOS_TEST_EQUAL(p[i], randNum);
    }

    auto b1 = Pothos::SharedBuffer::make(2048, 0/*node*/); //numa node 0 should always exist
    POTHOS_TEST_NOT_EQUAL(b1.getAddress(), 0);
    POTHOS_TEST_TRUE((b1.getAddress() & 0xf) == 0); //has alignment
    POTHOS_TEST_EQUAL(b1.getLength(), 2048);
    for (size_t i = 0; i < b1.getLength()/sizeof(int); i++)
    {
        int *p = reinterpret_cast<int *>(b1.getAddress());
        const int randNum = std::rand();
        p[i] = randNum;
        POTHOS_TEST_EQUAL(p[i], randNum);
    }

    POTHOS_TEST_THROWS(
        Pothos::SharedBuffer(b1.getAddress() + 512, b1.getLength(), b1),
        Pothos::SharedBufferError);
    auto b2 = Pothos::SharedBuffer(b1.getAddress() + 512, b1.getLength() - 512, b1);

    //test a size zero allocation
    POTHOS_TEST_CHECKPOINT();
    auto bEmpty = Pothos::SharedBuffer::make(0);
    POTHOS_TEST_EQUAL(bEmpty.getLength(), 0);
}

POTHOS_TEST_BLOCK("/framework/tests", test_circular_shared_buffer)
{
    auto b0 = Pothos::SharedBuffer::makeCirc(1024);
    POTHOS_TEST_NOT_EQUAL(b0.getAddress(), 0);
    POTHOS_TEST_TRUE((b0.getAddress() & 0xf) == 0); //has alignment
    POTHOS_TEST_GE(b0.getLength(), 1024);

    const size_t alias = b0.getLength()/sizeof(int);
    for (size_t i = 0; i < b0.getLength()/sizeof(int); i++)
    {
        int *p = reinterpret_cast<int *>(b0.getAddress());
        const int randNum = std::rand();
        p[i] = randNum;
        POTHOS_TEST_EQUAL(p[i], randNum);
        POTHOS_TEST_EQUAL(p[i+alias], randNum);
    }
}
