// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Util/Network.hpp>
#include <Poco/Net/SocketAddress.h>

static bool __isIPv6Supported(void)
{
    try
    {
        Poco::Net::SocketAddress("::", 0);
    }
    catch (const Poco::Exception &)
    {
        return false;
    }
    return true;
}

bool Pothos::Util::isIPv6Supported(void)
{
    static bool supported = __isIPv6Supported();
    return supported;
}

std::string Pothos::Util::getWildcardAddr(const std::string &service)
{
    std::string addr = isIPv6Supported()?"[::]":"0.0.0.0";
    if (not service.empty()) addr += ":" + service;
    return addr;
}

std::string Pothos::Util::getLoopbackAddr(const std::string &service)
{
    std::string addr = isIPv6Supported()?"[::1]":"127.0.0.1";
    if (not service.empty()) addr += ":" + service;
    return addr;
}
