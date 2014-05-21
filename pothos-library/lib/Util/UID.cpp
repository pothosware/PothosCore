// Copyright (c) 2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Util/UID.hpp>
#include <Poco/URI.h>
#include <Poco/Environment.h>
#include <Poco/Process.h>
#include <Poco/Format.h>

Pothos::Util::UID::UID(void):
    _uid(Poco::URI("pothos", Poco::Environment::nodeName(), Poco::format(
        "%s/%s/%s",
        Poco::Environment::nodeId(),
        std::to_string(Poco::Process::id()),
        std::to_string(size_t(this))
    )).toString())
{
    return;
}
