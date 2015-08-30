// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <iostream>

struct ManyIOsBlock : Pothos::Block
{
    ManyIOsBlock(const std::string &name)
    {
        this->setName(name);
        this->setupInput(0);
        this->setupOutput(0);
    }
};

POTHOS_TEST_BLOCK("/framework/tests", test_auto_ports)
{
    auto source = std::shared_ptr<ManyIOsBlock>(new ManyIOsBlock("source"));
    auto sink = std::shared_ptr<ManyIOsBlock>(new ManyIOsBlock("sink"));

    POTHOS_TEST_EQUAL(source->inputs().size(), 1);
    POTHOS_TEST_EQUAL(source->outputs().size(), 1);
    POTHOS_TEST_EQUAL(sink->inputs().size(), 1);
    POTHOS_TEST_EQUAL(sink->outputs().size(), 1);

    std::cout << "test input expansion" << std::endl;
    {
        Pothos::Topology topology;
        topology.connect(source, 0, sink, 0);
        topology.connect(source, 0, sink, 1);
        topology.connect(source, 0, sink, 2);
        POTHOS_TEST_EQUAL(source->outputs().size(), 1);
        POTHOS_TEST_EQUAL(sink->inputs().size(), 3);
    }

    POTHOS_TEST_EQUAL(source->outputs().size(), 1);
    POTHOS_TEST_EQUAL(sink->inputs().size(), 1);

    std::cout << "test output expansion" << std::endl;
    {
        Pothos::Topology topology;
        topology.connect(source, 0, sink, 0);
        topology.connect(source, 1, sink, 0);
        topology.connect(source, 2, sink, 0);
        POTHOS_TEST_EQUAL(source->outputs().size(), 3);
        POTHOS_TEST_EQUAL(sink->inputs().size(), 1);
    }

    POTHOS_TEST_EQUAL(source->outputs().size(), 1);
    POTHOS_TEST_EQUAL(sink->inputs().size(), 1);

    std::cout << "test both expansion" << std::endl;
    {
        Pothos::Topology topology;
        topology.connect(source, 0, sink, 2);
        topology.connect(source, 1, sink, 1);
        topology.connect(source, 2, sink, 0);
        POTHOS_TEST_EQUAL(source->outputs().size(), 3);
        POTHOS_TEST_EQUAL(sink->inputs().size(), 3);
    }

    POTHOS_TEST_EQUAL(source->outputs().size(), 1);
    POTHOS_TEST_EQUAL(sink->inputs().size(), 1);
}
