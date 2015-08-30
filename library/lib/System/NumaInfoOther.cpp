// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/System/NumaInfo.hpp>
#include <Poco/Environment.h>

//NUMA INFO Other assumes one NUMA node with all CPUs

std::vector<Pothos::System::NumaInfo> Pothos::System::NumaInfo::get(void)
{
    std::vector<NumaInfo> infoList;
    NumaInfo info;

    //a list of all the CPUs
    for (size_t i = 0; i < Poco::Environment::processorCount(); i++)
    {
        info.cpus.push_back(i);
    }

    //TODO query the available memory?

    infoList.push_back(info);
    return infoList;
}
