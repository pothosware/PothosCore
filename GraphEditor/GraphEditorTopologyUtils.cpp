// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "GraphEditor/GraphEditor.hpp"
#include "GraphObjects/GraphBlock.hpp"
#include "GraphObjects/GraphBreaker.hpp"
#include "GraphObjects/GraphConnection.hpp"

/*!
 * Given an input endpoint, discover all of the "resolved" input endpoints by traversing breakers of the same node name.
 */
static std::vector<GraphConnectionEndpoint> traverseInputEps(const GraphConnectionEndpoint &inputEp, const GraphObjectList &graphObjects)
{
    std::vector<GraphConnectionEndpoint> inputEndpoints;
    auto inputBlock = dynamic_cast<GraphBlock *>(inputEp.getObj().data());
    auto inputBreaker = dynamic_cast<GraphBreaker *>(inputEp.getObj().data());

    if (inputBlock != nullptr)
    {
        inputEndpoints.push_back(inputEp);
    }

    if (inputBreaker != nullptr)
    {
        auto nodeName = inputBreaker->getNodeName();
        for (auto graphObject : graphObjects)
        {
            auto breaker = dynamic_cast<GraphBreaker *>(graphObject);
            if (breaker == nullptr) continue;
            if (breaker->getNodeName() != nodeName) continue;
            if (breaker == inputBreaker) continue;
            //follow all connections from this breaker to an input
            //this is the recursive part
            for (auto graphSubObject : graphObjects)
            {
                auto connection = dynamic_cast<GraphConnection *>(graphSubObject);
                if (connection == nullptr) continue;
                if (connection->getOutputEndpoint().getObj() != breaker) continue;
                for (const auto &subEp : traverseInputEps(connection->getInputEndpoint(), graphObjects))
                {
                    inputEndpoints.push_back(subEp);
                }
            }
        }
    }

    return inputEndpoints;
}

static Poco::JSON::Array::Ptr getConnectionInfo(const GraphObjectList &graphObjects)
{
    Poco::JSON::Array::Ptr connections = new Poco::JSON::Array();
    for (auto graphObject : graphObjects)
    {
        auto connection = dynamic_cast<GraphConnection *>(graphObject);
        if (connection == nullptr) continue;
        auto outputEp = connection->getOutputEndpoint();
        auto inputEp = connection->getInputEndpoint();

        //ignore connections from output breakers
        //we will come back to them from the block to breaker to block path
        auto outputBreaker = dynamic_cast<GraphBreaker *>(outputEp.getObj().data());
        if (outputBreaker != nullptr) continue;

        for (const auto &subEp : traverseInputEps(inputEp, graphObjects))
        {
            Poco::JSON::Object::Ptr args = new Poco::JSON::Object();
            args->set("srcId", outputEp.getObj()->getId().toStdString());
            args->set("srcPort", outputEp.getKey().id.toStdString());
            args->set("dstId", subEp.getObj()->getId().toStdString());
            args->set("dstPort", subEp.getKey().id.toStdString());
            connections->add(args);
        }
    }
    return connections;
}

Poco::JSON::Array::Ptr GraphEditor::getConnectionInfo(void) const
{
    return ::getConnectionInfo(this->getGraphObjects());
}
