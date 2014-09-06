// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/Containers.hpp>

Pothos::MessageGram::MessageGram(void)
{
    return;
}

#include <Pothos/Managed.hpp>

static auto managedMessageGram = Pothos::ManagedClass()
    .registerConstructor<Pothos::MessageGram>()
    .registerField(POTHOS_FCN_TUPLE(Pothos::MessageGram, payload))
    .registerField(POTHOS_FCN_TUPLE(Pothos::MessageGram, metadata))
    .registerField(POTHOS_FCN_TUPLE(Pothos::MessageGram, labels))
    .commit("Pothos/MessageGram");

#include <Pothos/Object/Serialize.hpp>

namespace Pothos { namespace serialization {
template<class Archive>
void serialize(Archive & ar, Pothos::MessageGram &t, const unsigned int)
{
    //ar & t.payload;
    ar & t.metadata;
    //ar & t.labels;
}
}}

POTHOS_OBJECT_SERIALIZE(Pothos::MessageGram)
