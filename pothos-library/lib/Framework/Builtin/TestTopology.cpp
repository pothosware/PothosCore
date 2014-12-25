// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>

/***********************************************************************
 * Helper blocks to test the rendered flow of the topology
 **********************************************************************/
struct Ping : Pothos::Block
{
    Ping(const std::string which = ""):
        once(false)
    {
        this->setupOutput("out0");
        this->setName("Ping"+which);
    }

    void work(void)
    {
        if (once) return;
        once = true;
        auto out0 = this->output("out0");
        out0->postMessage(42);
    }

bool once;
};

struct Pong : Pothos::Block
{
    Pong(const std::string which = ""):
        triggered(0)
    {
        this->setupInput("in0");
        this->setName("Pong"+which);
    }

    void work(void)
    {
        auto in0 = this->input("in0");
        if (in0->hasMessage())
        {
            in0->popMessage();
            triggered++;
        }
    }

size_t triggered;
};

/***********************************************************************
 * Test a simple pass-through topology
 **********************************************************************/
POTHOS_TEST_BLOCK("/framework/tests/topology", test_simple_passthrough)
{
    //create the pass through topology
    auto passer = Pothos::Topology::make();
    passer->setName("Passer");
    passer->connect(passer, "passIn", passer, "passOut");

    //create ping pong blocks
    auto ping = std::shared_ptr<Ping>(new Ping());
    auto pong = std::shared_ptr<Pong>(new Pong());
    POTHOS_TEST_EQUAL(pong->triggered, 0);

    //connect all the blocks
    Pothos::Topology topology;
    topology.connect(ping, "out0", passer, "passIn");
    topology.connect(passer, "passOut", pong, "in0");
    topology.commit();

    //check that the message flowed
    POTHOS_TEST_TRUE(topology.waitInactive());
    POTHOS_TEST_EQUAL(pong->triggered, 1);

    //make sure we can disconnect everything
    topology.disconnect(ping, "out0", passer, "passIn");
    topology.disconnect(passer, "passOut", pong, "in0");
    topology.commit();
}

/***********************************************************************
 * Test a nested pass-through topology
 **********************************************************************/
POTHOS_TEST_BLOCK("/framework/tests/topology", test_nested_passthrough)
{
    //create the pass through topology
    auto passer = Pothos::Topology::make();
    passer->setName("Passer");
    passer->connect(passer, "passIn", passer, "passOut");

    //create and connect the nest topology
    auto nester = Pothos::Topology::make();
    nester->setName("Nester");
    nester->connect(nester, "nestIn", passer, "passIn");
    nester->connect(passer, "passOut", nester, "nestOut");

    //create ping pong blocks
    auto ping = std::shared_ptr<Ping>(new Ping());
    auto pong = std::shared_ptr<Pong>(new Pong());
    POTHOS_TEST_EQUAL(pong->triggered, 0);

    //connect all the blocks
    Pothos::Topology topology;
    topology.connect(ping, "out0", nester, "nestIn");
    topology.connect(nester, "nestOut", pong, "in0");
    topology.commit();

    //check that the message flowed
    POTHOS_TEST_TRUE(topology.waitInactive());
    POTHOS_TEST_EQUAL(pong->triggered, 1);
}

/***********************************************************************
 * Test a pass-through topology that terminates in different layers
 **********************************************************************/
POTHOS_TEST_BLOCK("/framework/tests/topology", test_uneven_passthrough)
{
    //create ping pong blocks
    auto ping = std::shared_ptr<Ping>(new Ping());
    auto pong = std::shared_ptr<Pong>(new Pong());

    //create the pass through topology
    auto passer = Pothos::Topology::make();
    passer->setName("Passer");
    passer->connect(passer, "passIn", passer, "passOut");

    //create and connect the nest topology
    auto nester = Pothos::Topology::make();
    nester->setName("Nester");
    nester->connect(ping, "out0", passer, "passIn");
    nester->connect(passer, "passOut", nester, "nestOut");

    //connect all the blocks
    Pothos::Topology topology;
    topology.connect(nester, "nestOut", pong, "in0");
    topology.commit();

    //check that the message flowed
    POTHOS_TEST_TRUE(topology.waitInactive());
    POTHOS_TEST_EQUAL(pong->triggered, 1);
}

/***********************************************************************
 * Test pass-through topology + multiple pass-through, multiple layers
 **********************************************************************/
POTHOS_TEST_BLOCK("/framework/tests/topology", test_multi_passthrough)
{
    //create the pass through topology
    auto passer = Pothos::Topology::make();
    passer->setName("Passer");
    passer->connect(passer, "passIn0", passer, "passOut0");
    passer->connect(passer, "passIn1", passer, "passOut1");

    //create and connect the nest topology
    auto nester = Pothos::Topology::make();
    nester->setName("Nester");
    nester->connect(nester, "nestIn", passer, "passIn0");
    nester->connect(passer, "passOut0", nester, "nestOut");
    auto pingInner = std::shared_ptr<Ping>(new Ping("Inner"));
    auto pongInner = std::shared_ptr<Pong>(new Pong("Inner"));
    nester->connect(pingInner, "out0", passer, "passIn1");
    nester->connect(passer, "passOut1", pongInner, "in0");

    //create ping pong blocks
    auto pingOuter = std::shared_ptr<Ping>(new Ping("Outer"));
    auto pongOuter = std::shared_ptr<Pong>(new Pong("Outer"));

    //connect all the blocks
    POTHOS_TEST_CHECKPOINT();
    Pothos::Topology topology;
    topology.connect(pingOuter, "out0", nester, "nestIn");
    topology.connect(nester, "nestOut", pongOuter, "in0");
    POTHOS_TEST_CHECKPOINT();
    topology.commit();

    //check that the message flowed
    POTHOS_TEST_TRUE(topology.waitInactive());
    POTHOS_TEST_EQUAL(pongInner->triggered, 1);
    POTHOS_TEST_EQUAL(pongOuter->triggered, 1);
}
