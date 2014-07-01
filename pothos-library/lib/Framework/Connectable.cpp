// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/Connectable.hpp>

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

#include <Pothos/Managed.hpp>

//FIXME see issue #37
static const std::string &getUid(const Pothos::Connectable &b)
{
    return b.uid();
}

static auto managedConnectable = Pothos::ManagedClass()
    .registerClass<Pothos::Connectable>()
    .registerMethod("uid", &getUid)
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Connectable, setName))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Connectable, getName))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Connectable, inputPortNames))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Connectable, outputPortNames))
    .commit("Pothos/Connectable");
