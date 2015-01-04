// Copyright (c) 2013-2013 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosUtil.hpp"
#include <Pothos/Framework.hpp>
#include <thread>
#include <iostream>

void PothosUtilBase::runTopology(const std::string &, const std::string &path)
{
    Pothos::init();

    std::cout << ">>> Create Topology: " << path << std::endl;
    auto topology = Pothos::Topology::make(path);

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
