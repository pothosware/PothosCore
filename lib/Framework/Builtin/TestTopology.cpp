// Copyright (c) 2014-2017 Josh Blum
//                    2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <iostream>
#include <json.hpp>

using json = nlohmann::json;

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
 * Helper blocks to test block w/ unconnected ports
 **********************************************************************/
struct Passer : Pothos::Block
{
    Passer(const std::string which = ""):
        workCount(0)
    {
        //forward in0 to out0
        //out0 to out1 are unused and should be unconnected
        this->setupInput("in0");
        this->setupInput("in1");
        this->setupOutput("out0");
        this->setupOutput("out1");
        this->setName("Passer"+which);
    }

    void work(void)
    {
        workCount++;
        auto in0 = this->input("in0");
        auto out0 = this->output("out0");
        if (in0->hasMessage())
        {
            auto msg = in0->popMessage();
            out0->postMessage(msg);
        }
    }

size_t workCount;
};

/***********************************************************************
 * Helper method for unit tests
 **********************************************************************/
static bool connectionsHave(
    const json &connsArray,
    const std::string &srcId, const std::string &srcName,
    const std::string &dstId, const std::string &dstName
)
{
    for (const auto &connObj : connsArray)
    {
        if (connObj["srcId"].get<std::string>() != srcId) continue;
        if (connObj["srcName"].get<std::string>() != srcName) continue;
        if (connObj["dstId"].get<std::string>() != dstId) continue;
        if (connObj["dstName"].get<std::string>() != dstName) continue;
        return true;
    }
    return false;
}

/***********************************************************************
 * Test operation with unused ports
 **********************************************************************/
POTHOS_TEST_BLOCK("/framework/tests/topology", test_unused_ports)
{
    //create blocks
    auto ping = std::shared_ptr<Ping>(new Ping());
    auto passer = std::shared_ptr<Passer>(new Passer());
    auto pong = std::shared_ptr<Pong>(new Pong());
    POTHOS_TEST_EQUAL(passer->workCount, 0);
    POTHOS_TEST_EQUAL(pong->triggered, 0);

    //connect all the blocks
    Pothos::Topology topology;
    topology.connect(ping, "out0", passer, "in0");
    topology.connect(passer, "out0", pong, "in0");
    topology.commit();

    //check that the message flowed
    POTHOS_TEST_TRUE(topology.waitInactive());
    POTHOS_TEST_NOT_EQUAL(passer->workCount, 0);
    POTHOS_TEST_EQUAL(pong->triggered, 1);
}

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

    //check the top JSON dump
    {
        POTHOS_TEST_CHECKPOINT();
        const auto topObj = json::parse(topology.dumpJSON("{\"mode\":\"top\"}"));
        auto connsArray = topObj["connections"];
        auto blocksObj = topObj["blocks"];
        POTHOS_TEST_EQUAL(blocksObj.size(), 3);
        POTHOS_TEST_EQUAL(connsArray.size(), 2);
        POTHOS_TEST_TRUE(blocksObj.count(passer->uid()));
        POTHOS_TEST_TRUE(blocksObj.count(ping->uid()));
        POTHOS_TEST_TRUE(blocksObj.count(pong->uid()));
        POTHOS_TEST_EQUAL(blocksObj[passer->uid()]["name"].get<std::string>(), "Passer");
        POTHOS_TEST_EQUAL(blocksObj[ping->uid()]["name"].get<std::string>(), "Ping");
        POTHOS_TEST_EQUAL(blocksObj[pong->uid()]["name"].get<std::string>(), "Pong");
        POTHOS_TEST_TRUE(connectionsHave(connsArray, ping->uid(), "out0", passer->uid(), "passIn"));
        POTHOS_TEST_TRUE(connectionsHave(connsArray, passer->uid(), "passOut", pong->uid(), "in0"));
    }

    //check the flat JSON dump
    {
        POTHOS_TEST_CHECKPOINT();
        const auto topObj = json::parse(topology.dumpJSON("{\"mode\":\"flat\"}"));
        auto connsArray = topObj["connections"];
        auto blocksObj = topObj["blocks"];
        POTHOS_TEST_EQUAL(blocksObj.size(), 2);
        POTHOS_TEST_EQUAL(connsArray.size(), 1);
        POTHOS_TEST_TRUE(blocksObj.count(ping->uid()));
        POTHOS_TEST_TRUE(blocksObj.count(pong->uid()));
        POTHOS_TEST_EQUAL(blocksObj[ping->uid()]["name"].get<std::string>(), "Ping");
        POTHOS_TEST_EQUAL(blocksObj[pong->uid()]["name"].get<std::string>(), "Pong");
        POTHOS_TEST_TRUE(connectionsHave(connsArray, ping->uid(), "out0", pong->uid(), "in0"));
    }

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

    //check the top JSON dump
    {
        POTHOS_TEST_CHECKPOINT();
        const auto topObj = json::parse(topology.dumpJSON("{\"mode\":\"top\"}"));
        auto connsArray = topObj["connections"];
        auto blocksObj = topObj["blocks"];
        POTHOS_TEST_EQUAL(blocksObj.size(), 3);
        POTHOS_TEST_EQUAL(connsArray.size(), 2);
        POTHOS_TEST_TRUE(blocksObj.count(nester->uid()));
        POTHOS_TEST_TRUE(blocksObj.count(ping->uid()));
        POTHOS_TEST_TRUE(blocksObj.count(pong->uid()));
        POTHOS_TEST_TRUE(connectionsHave(connsArray, ping->uid(), "out0", nester->uid(), "nestIn"));
        POTHOS_TEST_TRUE(connectionsHave(connsArray, nester->uid(), "nestOut", pong->uid(), "in0"));
    }

    //check the flat JSON dump
    {
        POTHOS_TEST_CHECKPOINT();
        const auto topObj = json::parse(topology.dumpJSON("{\"mode\":\"flat\"}"));
        auto connsArray = topObj["connections"];
        auto blocksObj = topObj["blocks"];
        POTHOS_TEST_EQUAL(blocksObj.size(), 2);
        POTHOS_TEST_EQUAL(connsArray.size(), 1);
        POTHOS_TEST_TRUE(blocksObj.count(ping->uid()));
        POTHOS_TEST_TRUE(blocksObj.count(pong->uid()));
        POTHOS_TEST_TRUE(connectionsHave(connsArray, ping->uid(), "out0", pong->uid(), "in0"));
    }
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

    //check the top JSON dump
    {
        POTHOS_TEST_CHECKPOINT();
        const auto topObj = json::parse(topology.dumpJSON("{\"mode\":\"top\"}"));
        auto connsArray = topObj["connections"];
        auto blocksObj = topObj["blocks"];
        POTHOS_TEST_EQUAL(blocksObj.size(), 2);
        POTHOS_TEST_EQUAL(connsArray.size(), 1);
        POTHOS_TEST_TRUE(blocksObj.count(nester->uid()));
        POTHOS_TEST_TRUE(blocksObj.count(pong->uid()));
        POTHOS_TEST_TRUE(connectionsHave(connsArray, nester->uid(), "nestOut", pong->uid(), "in0"));
    }

    //check the flat JSON dump
    {
        POTHOS_TEST_CHECKPOINT();
        const auto topObj = json::parse(topology.dumpJSON("{\"mode\":\"flat\"}"));
        auto connsArray = topObj["connections"];
        auto blocksObj = topObj["blocks"];
        POTHOS_TEST_EQUAL(blocksObj.size(), 2);
        POTHOS_TEST_EQUAL(connsArray.size(), 1);
        POTHOS_TEST_TRUE(blocksObj.count(ping->uid()));
        POTHOS_TEST_TRUE(blocksObj.count(pong->uid()));
        POTHOS_TEST_TRUE(connectionsHave(connsArray, ping->uid(), "out0", pong->uid(), "in0"));
    }
}

/***********************************************************************
 * Test pass-through topology
 * Multiple sources sharing a pass-through
 * The sources are at different layers
 **********************************************************************/
POTHOS_TEST_BLOCK("/framework/tests/topology", test_multisrc_passthrough)
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
    auto pingInner = std::shared_ptr<Ping>(new Ping("Inner"));
    nester->connect(pingInner, "out0", passer, "passIn");

    //create ping pong blocks
    auto pingOuter = std::shared_ptr<Ping>(new Ping("Outer"));
    auto pong = std::shared_ptr<Pong>(new Pong());

    //connect all the blocks
    POTHOS_TEST_CHECKPOINT();
    Pothos::Topology topology;
    topology.connect(pingOuter, "out0", nester, "nestIn");
    topology.connect(nester, "nestOut", pong, "in0");
    POTHOS_TEST_CHECKPOINT();
    topology.commit();

    //check that the message flowed
    POTHOS_TEST_TRUE(topology.waitInactive());
    POTHOS_TEST_EQUAL(pong->triggered, 2);

    //check the top JSON dump
    {
        POTHOS_TEST_CHECKPOINT();
        const auto topObj = json::parse(topology.dumpJSON("{\"mode\":\"top\"}"));
        auto connsArray = topObj["connections"];
        auto blocksObj = topObj["blocks"];
        POTHOS_TEST_EQUAL(blocksObj.size(), 3);
        POTHOS_TEST_EQUAL(connsArray.size(), 2);
        POTHOS_TEST_TRUE(blocksObj.count(nester->uid()));
        POTHOS_TEST_TRUE(blocksObj.count(pingOuter->uid()));
        POTHOS_TEST_TRUE(blocksObj.count(pong->uid()));
        POTHOS_TEST_TRUE(connectionsHave(connsArray, pingOuter->uid(), "out0", nester->uid(), "nestIn"));
        POTHOS_TEST_TRUE(connectionsHave(connsArray, nester->uid(), "nestOut", pong->uid(), "in0"));
    }

    //check the flat JSON dump
    {
        POTHOS_TEST_CHECKPOINT();
        const auto topObj = json::parse(topology.dumpJSON("{\"mode\":\"flat\"}"));
        auto connsArray = topObj["connections"];
        auto blocksObj = topObj["blocks"];
        POTHOS_TEST_EQUAL(blocksObj.size(), 3);
        POTHOS_TEST_EQUAL(connsArray.size(), 2);
        POTHOS_TEST_TRUE(blocksObj.count(pingOuter->uid()));
        POTHOS_TEST_TRUE(blocksObj.count(pingInner->uid()));
        POTHOS_TEST_TRUE(blocksObj.count(pong->uid()));
        POTHOS_TEST_TRUE(connectionsHave(connsArray, pingOuter->uid(), "out0", pong->uid(), "in0"));
        POTHOS_TEST_TRUE(connectionsHave(connsArray, pingInner->uid(), "out0", pong->uid(), "in0"));
    }
}

/***********************************************************************
 * Test pass-through topology
 * Multiple destinations sharing a pass-through
 * The destinations are at different layers
 **********************************************************************/
POTHOS_TEST_BLOCK("/framework/tests/topology", test_multidst_passthrough)
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
    auto pongInner = std::shared_ptr<Pong>(new Pong("Inner"));
    nester->connect(passer, "passOut", pongInner, "in0");

    //create ping pong blocks
    auto ping = std::shared_ptr<Ping>(new Ping());
    auto pongOuter = std::shared_ptr<Pong>(new Pong("Outer"));

    //connect all the blocks
    POTHOS_TEST_CHECKPOINT();
    Pothos::Topology topology;
    topology.connect(ping, "out0", nester, "nestIn");
    topology.connect(nester, "nestOut", pongOuter, "in0");
    POTHOS_TEST_CHECKPOINT();
    topology.commit();

    //check that the message flowed
    POTHOS_TEST_TRUE(topology.waitInactive());
    POTHOS_TEST_EQUAL(pongInner->triggered, 1);
    POTHOS_TEST_EQUAL(pongOuter->triggered, 1);

    //check the top JSON dump
    {
        POTHOS_TEST_CHECKPOINT();
        const auto topObj = json::parse(topology.dumpJSON("{\"mode\":\"top\"}"));
        auto connsArray = topObj["connections"];
        auto blocksObj = topObj["blocks"];
        POTHOS_TEST_EQUAL(blocksObj.size(), 3);
        POTHOS_TEST_EQUAL(connsArray.size(), 2);
        POTHOS_TEST_TRUE(blocksObj.count(nester->uid()));
        POTHOS_TEST_TRUE(blocksObj.count(pongOuter->uid()));
        POTHOS_TEST_TRUE(blocksObj.count(ping->uid()));
        POTHOS_TEST_TRUE(connectionsHave(connsArray, ping->uid(), "out0", nester->uid(), "nestIn"));
        POTHOS_TEST_TRUE(connectionsHave(connsArray, nester->uid(), "nestOut", pongOuter->uid(), "in0"));
    }

    //check the flat JSON dump
    {
        POTHOS_TEST_CHECKPOINT();
        const auto topObj = json::parse(topology.dumpJSON("{\"mode\":\"flat\"}"));
        auto connsArray = topObj["connections"];
        auto blocksObj = topObj["blocks"];
        POTHOS_TEST_EQUAL(blocksObj.size(), 3);
        POTHOS_TEST_EQUAL(connsArray.size(), 2);
        POTHOS_TEST_TRUE(blocksObj.count(pongOuter->uid()));
        POTHOS_TEST_TRUE(blocksObj.count(pongInner->uid()));
        POTHOS_TEST_TRUE(blocksObj.count(ping->uid()));
        POTHOS_TEST_TRUE(connectionsHave(connsArray, ping->uid(), "out0", pongOuter->uid(), "in0"));
        POTHOS_TEST_TRUE(connectionsHave(connsArray, ping->uid(), "out0", pongInner->uid(), "in0"));
    }
}

/***********************************************************************
 * Test pass-through topology
 * Multiple destinations and sources sharing a pass-through
 * The destinations and sources are at different layers
 **********************************************************************/
POTHOS_TEST_BLOCK("/framework/tests/topology", test_shared_passthrough)
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
    auto pingInner = std::shared_ptr<Ping>(new Ping("Inner"));
    auto pongInner = std::shared_ptr<Pong>(new Pong("Inner"));
    nester->connect(pingInner, "out0", passer, "passIn");
    nester->connect(passer, "passOut", pongInner, "in0");

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
    POTHOS_TEST_EQUAL(pongInner->triggered, 2);
    POTHOS_TEST_EQUAL(pongOuter->triggered, 2);

    //check the top JSON dump
    {
        POTHOS_TEST_CHECKPOINT();
        const auto topObj = json::parse(topology.dumpJSON("{\"mode\":\"top\"}"));
        auto connsArray = topObj["connections"];
        auto blocksObj = topObj["blocks"];
        POTHOS_TEST_EQUAL(blocksObj.size(), 3);
        POTHOS_TEST_EQUAL(connsArray.size(), 2);
        POTHOS_TEST_TRUE(blocksObj.count(nester->uid()));
        POTHOS_TEST_TRUE(blocksObj.count(pingOuter->uid()));
        POTHOS_TEST_TRUE(blocksObj.count(pingOuter->uid()));
        POTHOS_TEST_TRUE(connectionsHave(connsArray, pingOuter->uid(), "out0", nester->uid(), "nestIn"));
        POTHOS_TEST_TRUE(connectionsHave(connsArray, nester->uid(), "nestOut", pongOuter->uid(), "in0"));
    }

    //check the flat JSON dump
    {
        POTHOS_TEST_CHECKPOINT();
        const auto topObj = json::parse(topology.dumpJSON("{\"mode\":\"flat\"}"));
        auto connsArray = topObj["connections"];
        auto blocksObj = topObj["blocks"];
        POTHOS_TEST_EQUAL(blocksObj.size(), 4);
        POTHOS_TEST_EQUAL(connsArray.size(), 4);
        POTHOS_TEST_TRUE(blocksObj.count(pingInner->uid()));
        POTHOS_TEST_TRUE(blocksObj.count(pingOuter->uid()));
        POTHOS_TEST_TRUE(blocksObj.count(pingOuter->uid()));
        POTHOS_TEST_TRUE(blocksObj.count(pingOuter->uid()));
        POTHOS_TEST_TRUE(connectionsHave(connsArray, pingOuter->uid(), "out0", pongOuter->uid(), "in0"));
        POTHOS_TEST_TRUE(connectionsHave(connsArray, pingInner->uid(), "out0", pongInner->uid(), "in0"));
        POTHOS_TEST_TRUE(connectionsHave(connsArray, pingOuter->uid(), "out0", pongInner->uid(), "in0"));
        POTHOS_TEST_TRUE(connectionsHave(connsArray, pingInner->uid(), "out0", pongOuter->uid(), "in0"));
    }
}

/***********************************************************************
 * Test pass-through topology
 * Multiple destinations and sources with independent pass-throughs
 * The destinations and sources are at different layers
 **********************************************************************/
POTHOS_TEST_BLOCK("/framework/tests/topology", test_independent_passthrough)
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

    //check the top JSON dump
    {
        POTHOS_TEST_CHECKPOINT();
        const auto topObj = json::parse(topology.dumpJSON("{\"mode\":\"top\"}"));
        auto connsArray = topObj["connections"];
        auto blocksObj = topObj["blocks"];
        POTHOS_TEST_EQUAL(blocksObj.size(), 3);
        POTHOS_TEST_EQUAL(connsArray.size(), 2);
        POTHOS_TEST_TRUE(blocksObj.count(nester->uid()));
        POTHOS_TEST_TRUE(blocksObj.count(pingOuter->uid()));
        POTHOS_TEST_TRUE(blocksObj.count(pongOuter->uid()));
        POTHOS_TEST_TRUE(connectionsHave(connsArray, pingOuter->uid(), "out0", nester->uid(), "nestIn"));
        POTHOS_TEST_TRUE(connectionsHave(connsArray, nester->uid(), "nestOut", pongOuter->uid(), "in0"));
    }

    //check the flat JSON dump
    {
        POTHOS_TEST_CHECKPOINT();
        const auto topObj = json::parse(topology.dumpJSON("{\"mode\":\"flat\"}"));
        auto connsArray = topObj["connections"];
        auto blocksObj = topObj["blocks"];
        POTHOS_TEST_EQUAL(blocksObj.size(), 4);
        POTHOS_TEST_EQUAL(connsArray.size(), 2);
        POTHOS_TEST_TRUE(blocksObj.count(pingOuter->uid()));
        POTHOS_TEST_TRUE(blocksObj.count(pongOuter->uid()));
        POTHOS_TEST_TRUE(blocksObj.count(pingInner->uid()));
        POTHOS_TEST_TRUE(blocksObj.count(pongInner->uid()));
        POTHOS_TEST_TRUE(connectionsHave(connsArray, pingOuter->uid(), "out0", pongOuter->uid(), "in0"));
        POTHOS_TEST_TRUE(connectionsHave(connsArray, pingInner->uid(), "out0", pongInner->uid(), "in0"));
    }
}
