// Copyright (c) 2014-2014 Josh Blum
//                    2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/WorkInfo.hpp>

Pothos::WorkInfo::WorkInfo(void):
    minElements(0),
    minInElements(0),
    minOutElements(0),
    minAllElements(0),
    minAllInElements(0),
    minAllOutElements(0),
    maxTimeoutNs(0)
{
    return;
}

#include <Pothos/Managed.hpp>

static auto managedWorkInfo = Pothos::ManagedClass()
    .registerClass<Pothos::WorkInfo>()
    .registerField(POTHOS_FCN_TUPLE(Pothos::WorkInfo, minElements))
    .registerField(POTHOS_FCN_TUPLE(Pothos::WorkInfo, minInElements))
    .registerField(POTHOS_FCN_TUPLE(Pothos::WorkInfo, minOutElements))
    .registerField(POTHOS_FCN_TUPLE(Pothos::WorkInfo, minAllElements))
    .registerField(POTHOS_FCN_TUPLE(Pothos::WorkInfo, minAllInElements))
    .registerField(POTHOS_FCN_TUPLE(Pothos::WorkInfo, minAllOutElements))
    .registerField(POTHOS_FCN_TUPLE(Pothos::WorkInfo, maxTimeoutNs))
    .commit("Pothos/WorkInfo");

#include <Pothos/Plugin.hpp>
#include <Poco/Format.h>
#include <Poco/NumberFormatter.h>

static std::string workInfoToString(const Pothos::WorkInfo& workInfo)
{
    return Poco::format(
            "Pothos::WorkInfo (minElements: %s, minInElements: %s, minOutElements: %s, "
            "minAllElements: %s, minAllInElements: %s, minAllOutElements: %s, maxTimeoutNs: %s)",
            Poco::NumberFormatter::format(workInfo.minElements),
            Poco::NumberFormatter::format(workInfo.minInElements),
            Poco::NumberFormatter::format(workInfo.minOutElements),
            Poco::NumberFormatter::format(workInfo.minAllElements),
            Poco::NumberFormatter::format(workInfo.minAllInElements),
            Poco::NumberFormatter::format(workInfo.minAllOutElements),
            Poco::NumberFormatter::format(workInfo.maxTimeoutNs));
}

pothos_static_block(pothosRegisterPothosWorkInfoToString)
{
    Pothos::PluginRegistry::addCall(
        "/object/tostring/Pothos/WorkInfo",
        Pothos::Callable(&workInfoToString));
}
