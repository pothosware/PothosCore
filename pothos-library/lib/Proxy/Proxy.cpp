// Copyright (c) 2013-2013 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Proxy/Proxy.hpp>
#include <Pothos/Proxy/Handle.hpp>
#include <Pothos/Proxy/Exception.hpp>
#include <cassert>

Pothos::Proxy::Proxy(void)
{
    assert(this->null());
}

Pothos::Proxy::Proxy(const std::shared_ptr<ProxyHandle> &handle):
    _handle(handle)
{
    return;
}

Pothos::Proxy::Proxy(ProxyHandle *handle):
    _handle(handle)
{
    return;
}

std::shared_ptr<Pothos::ProxyHandle> Pothos::Proxy::getHandle(void) const
{
    return _handle;
}

std::shared_ptr<Pothos::ProxyEnvironment> Pothos::Proxy::getEnvironment(void) const
{
    assert(_handle);
    return _handle->getEnvironment();
}

bool Pothos::Proxy::null(void) const
{
    return not _handle;
}

int Pothos::Proxy::compareTo(const Proxy &other) const
{
    assert(_handle);
    return _handle->compareTo(other);
}

size_t Pothos::Proxy::hashCode(void) const
{
    assert(_handle);
    return _handle->hashCode();
}

std::string Pothos::Proxy::toString(void) const
{
    assert(_handle);
    return _handle->toString();
}

std::string Pothos::Proxy::getClassName(void) const
{
    assert(_handle);
    return _handle->getClassName();
}

bool Pothos::Proxy::operator<(const Pothos::Proxy &obj) const
{
    try
    {
        return this->compareTo(obj) < 0;
    }
    catch (const Pothos::ProxyCompareError &)
    {
        return this->hashCode() < obj.hashCode();
    }
}

bool Pothos::Proxy::operator>(const Pothos::Proxy &obj) const
{
    try
    {
        return this->compareTo(obj) > 0;
    }
    catch (const Pothos::ProxyCompareError &)
    {
        return this->hashCode() > obj.hashCode();
    }
}

bool Pothos::operator==(const Proxy &lhs, const Proxy &rhs)
{
    return lhs.getHandle() == rhs.getHandle();
}
