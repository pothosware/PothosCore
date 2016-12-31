///
/// \file Object/Serialize.hpp
///
/// Serialization support for types held by an Object.
/// This file provides the POTHOS_OBJECT_SERIALIZE macro
/// which exports types into the serialization system.
/// This file is not automatically included by Object.hpp;
/// users adding a type should explicitly include this file.
///
/// \copyright
/// Copyright (c) 2013-2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
/*!
 * Export a serialization for the specified type.
 * Call this macro only once in a cpp file.
 * Usage: POTHOS_OBJECT_SERIALIZE(MyType)
 */
#define POTHOS_OBJECT_SERIALIZE(ValueType) \
    POTHOS_CLASS_EXPORT_GUID(Pothos::Detail::ObjectContainerT<ValueType>, "Pothos::Object<" #ValueType ">")

#include <Pothos/Config.hpp>
#include <Pothos/Object/ObjectImpl.hpp>
#include <Pothos/Archive.hpp>
#include <cassert>

namespace Pothos {
namespace serialization {

template<class Archive>
void serialize(Archive &, Pothos::Detail::ObjectContainer &, const unsigned int){}

template <class Archive, typename ValueType>
void serialize(Archive &ar, Pothos::Detail::ObjectContainerT<ValueType> &t, const unsigned int)
{
    ar & t.value;
}

template<class Archive>
void save(Archive & ar, const Pothos::Object &t, const unsigned int)
{
    const bool is_null = not t;
    ar << is_null;
    if (not is_null) ar << t._impl;
}

template<class Archive>
void load(Archive & ar, Pothos::Object &t, const unsigned int)
{
    assert(not t);
    bool is_null = false;
    ar >> is_null;
    if (not is_null) ar >> t._impl;
}

} // namespace serialization
} // namespace Pothos

POTHOS_SERIALIZATION_SPLIT_FREE(Pothos::Object)
