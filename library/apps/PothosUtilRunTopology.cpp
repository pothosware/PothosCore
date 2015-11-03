// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosUtil.hpp"
#include <Pothos/Framework.hpp>
#include <Poco/Path.h>
#include <fstream>
#include <thread>
#include <iostream>

void PothosUtilBase::runTopology(void)
{
    Pothos::ScopedInit init;

    //read the complete file into a string
    const auto path = this->config().getString("inputFile");
    std::ifstream ifs(Poco::Path::expand(path));
    const std::string json((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

    //create the topology from the JSON string
    std::cout << ">>> Create Topology: " << path << std::endl;
    auto topology = Pothos::Topology::make(json);

    //commit the topology and wait for specified time for CTRL+C
    if (this->config().has("runDuration"))
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
