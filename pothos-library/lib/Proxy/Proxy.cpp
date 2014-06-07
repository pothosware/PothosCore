// Copyright (c) 2013-2013 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Proxy/Proxy.hpp>
#include <Pothos/Proxy/Handle.hpp>
#include <Pothos/Proxy/Exception.hpp>
#include <cassert>

Pothos::Proxy::Proxy(void)
{
    assert(not *this);
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

Pothos::Proxy::operator bool(void) const
{
    return bool(_handle);
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

#include <Pothos/Object/Serialize.hpp>
#include <Pothos/Proxy.hpp>
#include <sstream>

namespace Pothos { namespace serialization {
template<class Archive>
void save(Archive & ar, const Pothos::Proxy &t, const unsigned int)
{
    auto name = t.getEnvironment()->getName();
    ar << name;
    std::ostringstream oss;
    t.getEnvironment()->serialize(t, oss);
    auto data = oss.str();
    ar << data;
}

template<class Archive>
void load(Archive & ar, Pothos::Proxy &t, const unsigned int)
{
    std::string name;
    ar >> name;
    auto env = Pothos::ProxyEnvironment::make(name);
    std::string data;
    ar >> data;
    std::istringstream iss(data);
    t = env->deserialize(iss);
}
}}

POTHOS_SERIALIZATION_SPLIT_FREE(Pothos::Proxy)
POTHOS_OBJECT_SERIALIZE(Pothos::Proxy)
