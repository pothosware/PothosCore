// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/Topology.hpp>
#include <Pothos/Proxy.hpp>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>
#include <Poco/File.h>
#include <fstream>
#include <map>

std::shared_ptr<Pothos::Topology> Pothos::Topology::make(const std::string &json)
{
    //parse the json string/file to a JSON object
    Poco::JSON::Object::Ptr topObj;
    Poco::JSON::Parser p;
    if (Poco::File(json).exists())
    {
        std::ifstream ifs(json);
        p.parse(ifs);
    }
    else
    {
        p.parse(json);
    }
    topObj = p.getHandler()->asVar().extract<Poco::JSON::Object::Ptr>();

    //create the blocks
    std::map<std::string, Pothos::Proxy> blocks;
    Poco::JSON::Array::Ptr blockArray;
    if (topObj->isArray("blocks")) blockArray = topObj->getArray("blocks");
    for (size_t i = 0; i < blockArray->size(); i++)
    {
        const auto &blockObj = blockArray->getObject(i);
    }

    //create the topology and connect the blocks
    auto topology = Pothos::Topology::make();
    Poco::JSON::Array::Ptr connArray;
    if (topObj->isArray("connections")) connArray = topObj->getArray("connections");
    for (size_t i = 0; i < connArray->size(); i++)
    {
        const auto &connObj = connArray->getObject(i);
    }

    return topology;
}
