// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework/BufferManager.hpp>
#include <Pothos/Framework/BufferChunk.hpp>
#include <Pothos/Framework/Exception.hpp>

POTHOS_TEST_BLOCK("/framework/tests", test_circular_buffer_manager)
{
    Pothos::BufferManagerArgs args;
    args.numBuffers = 2;
    auto manager = Pothos::BufferManager::make("circular", args);
    POTHOS_TEST_TRUE(not manager->empty());

    std::vector<Pothos::BufferChunk> buffs(2);
    buffs[0] = manager->front();
    manager->pop(buffs[0].length);
    POTHOS_TEST_TRUE(not manager->empty());
    buffs[1] = manager->front();
    manager->pop(buffs[1].length);
    POTHOS_TEST_TRUE(manager->empty());
    POTHOS_TEST_TRUE(not (buffs[0].getManagedBuffer() == buffs[1].getManagedBuffer()));

    //causes push in wrong order
    buffs[1] = Pothos::BufferChunk();
    buffs[0] = Pothos::BufferChunk();
    POTHOS_TEST_TRUE(not manager->empty());

    //and do it again
    buffs[0] = manager->front();
    manager->pop(buffs[0].length);
    buffs[1] = manager->front();
    manager->pop(buffs[1].length);
    POTHOS_TEST_TRUE(manager->empty());
    POTHOS_TEST_TRUE(not (buffs[0].getManagedBuffer() == buffs[1].getManagedBuffer()));

    //run through the buffers to cycle through everything
    buffs.clear(); //release any claimed buffers
    for (size_t i = 0; i < 100; i++)
    {
        auto buff = manager->front();
        manager->pop(buff.getBuffer().getLength());
    }
}
