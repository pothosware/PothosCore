// Copyright (c) 2013-2014 Josh Blum
//                    2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework/BufferManager.hpp>
#include <Pothos/Framework/BufferChunk.hpp>
#include <Pothos/Framework/Exception.hpp>

POTHOS_TEST_BLOCK("/framework/tests", test_generic_buffer_manager)
{
    Pothos::BufferManagerArgs args;
    args.numBuffers = 2;
    auto manager = Pothos::BufferManager::make("generic", args);
    POTHOS_TEST_FALSE(manager->empty());

    std::vector<Pothos::BufferChunk> buffs(2);
    buffs[0] = manager->front();
    manager->pop(buffs[0].length);
    buffs[1] = manager->front();
    manager->pop(buffs[1].length);
    POTHOS_TEST_TRUE(manager->empty());
    POTHOS_TEST_NOT_EQUAL(buffs[0].getManagedBuffer(), buffs[1].getManagedBuffer());

    buffs.clear();
    POTHOS_TEST_FALSE(manager->empty());
}
