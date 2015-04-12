// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <iostream>

struct SourceBlock : Pothos::Block
{
    SourceBlock(void)
    {
        this->setupOutput("out0");
    }
};

struct PassBlock : Pothos::Block
{
    PassBlock(void)
    {
        this->setupInput("in0");
        this->setupOutput("out0");
    }
};

struct SinkBlock : Pothos::Block
{
    SinkBlock(void)
    {
        this->setupInput("in0");
    }
};

POTHOS_TEST_BLOCK("/framework/tests/topology", test_dump_json)
{
    Pothos::Topology topology;

    auto b0 = std::shared_ptr<Pothos::Block>(new SourceBlock());
    b0->setName("b0");
    auto b1 = std::shared_ptr<Pothos::Block>(new SinkBlock());
    b1->setName("b1");

    topology.connect(b0, "out0", b1, "in0");

    std::cout << topology.dumpJSON() << std::endl;
}

POTHOS_TEST_BLOCK("/framework/tests/topology", test_dump_json2)
{
    Pothos::Topology topology;

    auto src0 = std::shared_ptr<Pothos::Block>(new SourceBlock());
    src0->setName("src0");
    auto sink0 = std::shared_ptr<Pothos::Block>(new SinkBlock());
    sink0->setName("sink0");
    auto pass0 = std::shared_ptr<Pothos::Block>(new PassBlock());
    pass0->setName("pass0");

    auto subTopology = Pothos::Topology::make();
    subTopology->setName("subTopology0");
    subTopology->connect(subTopology, "in0", pass0, "in0");
    subTopology->connect(pass0, "out0", subTopology, "out0");

    topology.connect(src0, "out0", subTopology, "in0");
    topology.connect(subTopology, "out0", sink0, "in0");

    std::cout << topology.dumpJSON() << std::endl;
}
