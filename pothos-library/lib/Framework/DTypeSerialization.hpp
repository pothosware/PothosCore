// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Framework/DType.hpp>
#include <Pothos/Object/Serialize.hpp>

namespace Pothos { namespace serialization {
template<class Archive>
void save(Archive & ar, const Pothos::DType &t, const unsigned int)
{
    ar << t.name();
    ar << t.shape();
    size_t elemSize = t.elemSize();
    ar << elemSize;
}

template<class Archive>
void load(Archive & ar, Pothos::DType &t, const unsigned int)
{
    std::string name;
    Pothos::DType::Shape shape;
    size_t elemSize;
    ar >> name;
    ar >> shape;
    ar >> elemSize;
    t = Pothos::DType(name, elemSize, shape);
}
}}

POTHOS_SERIALIZATION_SPLIT_FREE(Pothos::DType)
