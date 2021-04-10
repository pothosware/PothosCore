// Copyright (c) 2021 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include "Testing/ScopedBlockInRegistry.hpp"

#include <Pothos/Framework.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/Testing.hpp>

#include <string>

class TestBlock: public Pothos::Block
{
public:
    static Pothos::Block* make()
    {
        return new TestBlock();
    }
};

POTHOS_TEST_BLOCK("/framework/tests", test_does_block_exist)
{
    const std::string blockPath = "/tests/does_block_exist/test_path";

    {
        ScopedBlockInRegistry blockInRegistry(blockPath, &TestBlock::make);
        POTHOS_TEST_TRUE(Pothos::BlockRegistry::doesBlockExist(blockPath));
    }
    POTHOS_TEST_FALSE(Pothos::BlockRegistry::doesBlockExist(blockPath));
}
