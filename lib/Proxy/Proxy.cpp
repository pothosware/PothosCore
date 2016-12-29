// Copyright (c) 2013-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Proxy/Proxy.hpp>
#include <Pothos/Proxy/Handle.hpp>
#include <Pothos/Proxy/Environment.hpp>
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

Pothos::Proxy Pothos::Proxy::get(const std::string &name) const
{
    return this->callProxy("get:"+name);
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

Pothos::Object Pothos::Proxy::toObject(void) const
{
    assert(_handle);
    return _handle->getEnvironment()->convertProxyToObject(*this);
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
#include <Poco/Types.h>
#include <sstream>
#include <vector>

namespace Pothos { namespace serialization {
template<class Archive>
void save(Archive & ar, const Pothos::Proxy &t, const unsigned int)
{
    auto name = t.getEnvironment()->getName();
    ar << name;

    //serialize to stringstream
    std::stringstream ss;
    t.getEnvironment()->serialize(t, ss);
    const auto buff = ss.str();

    //save length and buffer
    const Poco::UInt32 length = Poco::UInt32(buff.size());
    ar << length;
    Pothos::serialization::BinaryObject bo(buff.data(), buff.size());
    ar << bo;
}

template<class Archive>
void load(Archive & ar, Pothos::Proxy &t, const unsigned int)
{
    std::string name;
    ar >> name;
    auto env = Pothos::ProxyEnvironment::make(name);

    //extract length and buffer
    Poco::UInt32 length = 0;
    ar >> length;
    auto buff = std::vector<char>(size_t(length));
    Pothos::serialization::BinaryObject bo(buff.data(), buff.size());
    ar >> bo;

    //deserialize from stringstream
    std::stringstream ss;
    ss.write((const char *)buff.data(), buff.size());
    t = env->deserialize(ss);
}
}}

POTHOS_SERIALIZATION_SPLIT_FREE(Pothos::Proxy)
POTHOS_OBJECT_SERIALIZE(Pothos::Proxy)
POTHOS_OBJECT_SERIALIZE(Pothos::ProxyVector)
POTHOS_OBJECT_SERIALIZE(Pothos::ProxySet)
POTHOS_OBJECT_SERIALIZE(Pothos::ProxyMap)
