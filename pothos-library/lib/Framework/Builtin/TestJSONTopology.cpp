// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <iostream>

struct TestBlock0 : Pothos::Block
{
    TestBlock0(void)
    {
        this->setupOutput("out0");
    }
};

struct TestBlock1 : Pothos::Block
{
    TestBlock1(void)
    {
        this->setupInput("in0");
    }
};

POTHOS_TEST_BLOCK("/framework/tests/topology", test_dump_json)
{
    Pothos::Topology topology;

    auto b0 = std::shared_ptr<Pothos::Block>(new TestBlock0());
    b0->setName("b0");
    auto b1 = std::shared_ptr<Pothos::Block>(new TestBlock1());
    b1->setName("b1");

    topology.connect(b0, "out0", b1, "in0");

    std::cout << topology.dumpJSON() << std::endl;
}
