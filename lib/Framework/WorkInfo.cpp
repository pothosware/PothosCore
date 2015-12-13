// Copyright (c) 2014-2014 Josh Blum
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
