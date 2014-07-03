// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Poco/JSON/Array.h>
#include "BlockEval.hpp"
#include <string>
#include <map>

class TopologyEngine
{
public:
    TopologyEngine(void)
    {
        return;
    }

    void acceptBlock(const BlockEval &blockEval)
    {
        auto proxy = blockEval.getProxyBlock();
        _idToBlock[proxy.call<std::string>("name")] = proxy;
    }

    void commitUpdate(const Poco::JSON::Array::Ptr &connections)
    {
        _topology.disconnectAll();
        for (size_t i = 0; i < connections->size(); i++)
        {
            auto connection = connections->getObject(i);
            _topology.connect(
                _idToBlock[connection->getValue<std::string>("srcId")],
                connection->getValue<std::string>("dstPort"),
                _idToBlock[connection->getValue<std::string>("dstId")],
                connection->getValue<std::string>("dstPort"));
        }
        _topology.commit();
        _idToBlock.clear();
    }

    std::string toDotMarkup(void)
    {
        return _topology.toDotMarkup();
    }
private:
    std::map<std::string, Pothos::Proxy> _idToBlock;
    Pothos::Topology _topology;
};

#include <Pothos/Managed.hpp>

static auto managedTopologyEngine = Pothos::ManagedClass()
    .registerConstructor<TopologyEngine>()
    .registerMethod(POTHOS_FCN_TUPLE(TopologyEngine, acceptBlock))
    .registerMethod(POTHOS_FCN_TUPLE(TopologyEngine, commitUpdate))
    .registerMethod(POTHOS_FCN_TUPLE(TopologyEngine, toDotMarkup))
    .commit("Pothos/Gui/TopologyEngine");
