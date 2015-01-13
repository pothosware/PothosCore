// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosUtil.hpp"
#include <Pothos/Framework.hpp>
#include <Poco/Path.h>
#include <fstream>
#include <thread>
#include <iostream>

void PothosUtilBase::runTopology(const std::string &, const std::string &path)
{
    Pothos::init();

    //read the complete file into a string
    std::ifstream ifs(Poco::Path::expand(path));
    const std::string json((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

    std::cout << ">>> Create Topology: " << path << std::endl;
    auto topology = Pothos::Topology::make(json);

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
}
