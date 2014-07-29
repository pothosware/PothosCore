// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/Connectable.hpp>

Pothos::PortInfo::PortInfo(void):
    isSpecial(false)
{
    return;
}

Pothos::Connectable::~Connectable(void)
{
    return;
}

void Pothos::Connectable::setName(const std::string &name)
{
    _name = name;
}

const std::string &Pothos::Connectable::getName(void) const
{
    return _name;
}

std::vector<std::string> Pothos::Connectable::inputPortNames(void)
{
    std::vector<std::string> names;
    for (const auto &info : this->inputPortInfo()) names.push_back(info.name);
    return names;
}

std::vector<std::string> Pothos::Connectable::outputPortNames(void)
{
    std::vector<std::string> names;
    for (const auto &info : this->outputPortInfo()) names.push_back(info.name);
    return names;
}

#include <Pothos/Managed.hpp>

static auto managedConnectable = Pothos::ManagedClass()
    .registerClass<Pothos::Connectable>()
    .registerBaseClass<Pothos::Connectable, Pothos::Util::UID>()
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Connectable, setName))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Connectable, getName))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Connectable, inputPortInfo))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Connectable, outputPortInfo))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Connectable, inputPortNames))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Connectable, outputPortNames))
    .commit("Pothos/Connectable");

#include <Pothos/Object/Serialize.hpp>
#include "Framework/DTypeSerialization.hpp"

namespace Pothos { namespace serialization {
template<class Archive>
void serialize(Archive & ar, Pothos::PortInfo &t, const unsigned int)
{
    ar & t.name;
    ar & t.isSpecial;
    ar & t.dtype;
}
}}

POTHOS_OBJECT_SERIALIZE(Pothos::PortInfo)
POTHOS_OBJECT_SERIALIZE(std::vector<Pothos::PortInfo>)
