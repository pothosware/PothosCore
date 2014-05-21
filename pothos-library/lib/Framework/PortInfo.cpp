// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/PortInfo.hpp>

Pothos::PortInfo::PortInfo(void)
{
    return;
}

Pothos::PortInfo::PortInfo(const std::string &name, const DType &dtype):
    _name(name), _dtype(dtype)
{
    return;
}

const std::string &Pothos::PortInfo::name(void) const
{
    return _name;
}

const Pothos::DType &Pothos::PortInfo::dtype(void) const
{
    return _dtype;
}

#include <Pothos/Object/Serialize.hpp>

namespace Pothos { namespace serialization {
template<class Archive>
void save(Archive & ar, const Pothos::PortInfo &t, const unsigned int)
{
    ar << t.name();
    Pothos::Object d(t.dtype());
    ar << d;
}

template<class Archive>
void load(Archive & ar, Pothos::PortInfo &t, const unsigned int)
{
    std::string name;
    ar >> name;
    Pothos::Object d;
    ar >> d;
    t = Pothos::PortInfo(name, d.extract<Pothos::DType>());
}
}}

POTHOS_SERIALIZATION_SPLIT_FREE(Pothos::PortInfo)
POTHOS_OBJECT_SERIALIZE(Pothos::PortInfo)
POTHOS_OBJECT_SERIALIZE(std::vector<Pothos::PortInfo>)

#include <Pothos/Managed.hpp>

static auto managedWorkerPortInfo = Pothos::ManagedClass()
    .registerConstructor<Pothos::PortInfo>()
    .registerConstructor<Pothos::PortInfo, const std::string &, const Pothos::DType &>()
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::PortInfo, name))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::PortInfo, dtype))
    .commit("Pothos/PortInfo");
