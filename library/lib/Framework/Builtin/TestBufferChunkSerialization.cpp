// Copyright (c) 2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Object.hpp>
#include <Pothos/Framework/BufferChunk.hpp>
#include <sstream>
#include <cstdlib> //rand

POTHOS_TEST_BLOCK("/framework/tests", test_buffer_chunk_serialization)
{
    const size_t numElems = 10000;
    Pothos::BufferChunk inputBuffer(sizeof(int)*numElems);
    for (size_t i = 0; i < numElems; i++)
    {
        inputBuffer.as<int *>()[i] = int(std::rand());
    }

    std::stringstream ss;
    Pothos::Object(inputBuffer).serialize(ss);

    Pothos::Object out;
    out.deserialize(ss);
    POTHOS_TEST_TRUE(out.type() == typeid(Pothos::BufferChunk));
    auto outputBuffer = out.extract<Pothos::BufferChunk>();

    POTHOS_TEST_EQUAL(inputBuffer.length, outputBuffer.length);
    for (size_t i = 0; i < numElems; i++)
    {
        POTHOS_TEST_EQUAL(inputBuffer.as<int *>()[i], outputBuffer.as<int *>()[i]);
    }
}
