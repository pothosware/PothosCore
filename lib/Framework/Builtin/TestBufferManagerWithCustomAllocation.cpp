// Copyright (c) 2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework/BufferManager.hpp>
#include <Pothos/Framework/BufferChunk.hpp>
#include <Pothos/Framework/Exception.hpp>

#include <memory>
#include <vector>

POTHOS_TEST_BLOCK("/framework/tests", test_buffer_manager_with_custom_allocation)
{
    bool wasCustomAllocatorUsed = false;
    std::shared_ptr<std::vector<uint8_t>> allocatorVecSPtr;

    auto customAllocator = [&](const Pothos::BufferManagerArgs& args)
    {
        allocatorVecSPtr.reset(new std::vector<uint8_t>(args.bufferSize*args.numBuffers));
        wasCustomAllocatorUsed = true;

        return Pothos::SharedBuffer(
                   reinterpret_cast<size_t>(allocatorVecSPtr->data()),
                   allocatorVecSPtr->size(),
                   allocatorVecSPtr);
    };

    Pothos::BufferManagerArgs args;
    args.bufferSize = 4096;
    args.numBuffers = 8;

    // This initializes, so our variables captured by the allocator lambda should
    // be affected.
    auto manager = Pothos::BufferManager::make("generic", args, customAllocator);
    POTHOS_TEST_FALSE(manager->empty());
    POTHOS_TEST_TRUE(wasCustomAllocatorUsed);
    POTHOS_TEST_TRUE(allocatorVecSPtr.get());
    POTHOS_TEST_EQUAL((args.bufferSize*args.numBuffers), allocatorVecSPtr->size());

    std::vector<Pothos::BufferChunk> managerBuffs;
    while(!manager->empty())
    {
        managerBuffs.emplace_back(manager->front());
        POTHOS_TEST_EQUAL(args.bufferSize, managerBuffs.back().length);

        manager->pop(args.bufferSize);
    }
    POTHOS_TEST_EQUAL(args.numBuffers, managerBuffs.size());

    for(size_t bufferIndex = 0; bufferIndex < args.numBuffers; ++bufferIndex)
    {
        POTHOS_TEST_EQUAL(
            reinterpret_cast<size_t>(&allocatorVecSPtr->at(args.bufferSize*bufferIndex)),
            managerBuffs[bufferIndex].address);
    }
}
