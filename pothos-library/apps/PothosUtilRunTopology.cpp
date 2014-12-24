// Copyright (c) 2013-2013 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosUtil.hpp"
#include <Pothos/Framework.hpp>
#include <iostream>

void PothosUtilBase::runTopology(const std::string &, const std::string &path)
{
    Pothos::init();

    std::cout << "Create Topology: " << path << std::endl;
    auto topology = Pothos::Topology::make(path);

    std::cout << "Running topology, press CTRL+C to exit" << path << std::endl;
    topology->commit();
    this->waitForTerminationRequest();
}
