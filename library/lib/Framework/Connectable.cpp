// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/Connectable.hpp>
#include <Pothos/Framework/Exception.hpp>

Pothos::PortInfo::PortInfo(void):
    isSigSlot(false)
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

Pothos::Object Pothos::Connectable::opaqueCall(const Object *inputArgs, const size_t numArgs) const
{
    if (numArgs == 0 or not inputArgs[0].canConvert(typeid(std::string)))
    {
        throw Pothos::BlockCallNotFound("Pothos::Connectable::call()", "missing method name");
    }
    return this->opaqueCallMethod(inputArgs[0].convert<std::string>(), inputArgs+1, numArgs-1);
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
    .registerWildcardMethod(&Pothos::Connectable::opaqueCallMethod)
    .commit("Pothos/Connectable");

#include <Pothos/Object/Serialize.hpp>

namespace Pothos { namespace serialization {
template<class Archive>
void serialize(Archive & ar, Pothos::PortInfo &t, const unsigned int)
{
    ar & t.name;
    ar & t.alias;
    ar & t.isSigSlot;
    ar & t.dtype;
}
}}

POTHOS_OBJECT_SERIALIZE(Pothos::PortInfo)
POTHOS_OBJECT_SERIALIZE(std::vector<Pothos::PortInfo>)
