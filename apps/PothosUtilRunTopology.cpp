// Copyright (c) 2014-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosUtil.hpp"
#include <Pothos/Framework.hpp>
#include <Pothos/Exception.hpp>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Path.h>
#include <fstream>
#include <thread>
#include <sstream>
#include <iostream>

void PothosUtilBase::runTopology(void)
{
    Pothos::ScopedInit init;

    //sanity check the file
    const auto path = this->config().getString("inputFile");
    if (Poco::Path(path).getExtension() == "pothos" or
        Poco::Path(path).getExtension() == "pth")
    {
        throw Pothos::DataFormatException("Cannot load "+path+"!\n"
            "Please export the design to the JSON topology format.");
    }
    std::ifstream ifs(Poco::Path::expand(path));
    if (not ifs) throw Pothos::FileException("Cant open "+path+" for reading!");

    //parse the json formatted string into a JSON object
    Poco::JSON::Object::Ptr topObj;
    try
    {
        const auto result = Poco::JSON::Parser().parse(ifs);
        topObj = result.extract<Poco::JSON::Object::Ptr>();
    }
    catch (const Poco::Exception &ex)
    {
        throw Pothos::DataFormatException(ex.message());
    }

    //apply the global variable overlays
    if (not topObj->isArray("globals")) topObj->set("globals",
        Poco::JSON::Array::Ptr(new Poco::JSON::Array()));
    auto globalsArray = topObj->getArray("globals");
    for (const auto &pair : _vars)
    {
        //look for a match in the existing globals and override its value
        for (size_t i = 0; i < globalsArray->size(); i++)
        {
            if (not globalsArray->isObject(i)) continue;
            auto globalVarObj = globalsArray->getObject(i);
            if (globalVarObj->optValue<std::string>("name", "") == pair.first)
            {
                globalVarObj->remove("value");
                globalVarObj->set("value", pair.second);
                goto nextVar;
            }
        }

        //otherwise add to the end of the globals array
        {
            std::cout << "add name " << pair.first << std::endl;
            Poco::JSON::Object::Ptr globalVarObj(new Poco::JSON::Object());
            globalVarObj->set("name", pair.first);
            globalVarObj->set("value", pair.second);
            globalsArray->add(globalVarObj);
        }

        nextVar: continue;
    }

    //create the topology from the JSON string
    std::cout << ">>> Create Topology: " << path << std::endl;
    std::stringstream ss; topObj->stringify(ss);
    auto topology = Pothos::Topology::make(ss.str());

    //commit the topology and wait for specified time for CTRL+C
    if (this->config().has("idleTime"))
    {
        const auto idleTime = this->config().getDouble("idleTime");
        double timeout = 0.0;
        std::string timeoutMsg = "no timeout";
        if (this->config().has("runDuration"))
        {
            timeout = this->config().getDouble("runDuration");
            timeoutMsg = "timeout " + std::to_string(timeout) + " seconds";
        }
        std::cout << ">>> Running topology until idle with " << timeoutMsg << std::endl;
        topology->commit();
        if (not topology->waitInactive(idleTime, timeout))
        {
            throw Pothos::RuntimeException("Topology::waitInactive() reached timeout");
        }
    }
    else if (this->config().has("runDuration"))
    {
        const auto runDuration = this->config().getDouble("runDuration");
        std::cout << ">>> Running topology for " << runDuration << " seconds" << std::endl;
        topology->commit();
        std::this_thread::sleep_for(std::chrono::milliseconds(long(runDuration*1000)));
    }
    else
    {
        std::cout << ">>> Running topology, press CTRL+C to exit" << std::endl;
        topology->commit();
        this->waitForTerminationRequest();
    }

    //dump the stats to file if specified
    if (this->config().has("outputFile"))
    {
        const auto statsFile = this->config().getString("outputFile");
        std::cout << ">>> Dumping stats: " << statsFile << std::endl;
        std::ofstream ofs(Poco::Path::expand(statsFile));
        ofs << topology->queryJSONStats() << std::endl;
    }
}
