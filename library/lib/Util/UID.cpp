// Copyright (c) 2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Util/UID.hpp>
#include <Poco/URI.h>
#include <Poco/Environment.h>
#include <Poco/Process.h>
#include <Poco/Format.h>
#include <atomic>

static std::atomic<unsigned long long> count;

Pothos::Util::UID::UID(void):
    _uid(Poco::URI("pothos", Poco::Environment::nodeName(), Poco::format(
        "%s/%s/%s",
        Poco::Environment::nodeId(),
        std::to_string(Poco::Process::id()),
        std::to_string(count.fetch_add(1, std::memory_order_relaxed))
    )).toString())
{
    return;
}

#include <Pothos/Managed.hpp>

static auto managedUID = Pothos::ManagedClass()
    .registerConstructor<Pothos::Util::UID>()
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Util::UID, uid))
    .commit("Pothos/Util/UID");
