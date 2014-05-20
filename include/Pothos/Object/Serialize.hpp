//
// Object/Serialize.hpp
//
// Serialization support for types held by an Object.
// This file provides the POTHOS_OBJECT_SERIALIZE macro
// which exports types into the serialization system.
// This file is not automatically included by Object.hpp;
// users adding a type should explicitly include this file.
//
// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

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
#include <cassert>

//disable auto linking
#ifndef POTHOS_ALL_NO_LIB
#define POTHOS_ALL_NO_LIB
#endif

#include <Pothos/serialization/base_object.hpp>
#include <Pothos/serialization/split_free.hpp>

#include <Pothos/archive/polymorphic_iarchive.hpp>
#include <Pothos/archive/polymorphic_oarchive.hpp>

#include <Pothos/serialization/export.hpp>
#include <Pothos/serialization/extended_type_info.hpp>

namespace Pothos {
namespace serialization {

template<class Archive>
void serialize(Archive &, Pothos::Detail::ObjectContainer &, const unsigned int){}

template <class Archive, typename ValueType>
void serialize(Archive &ar, Pothos::Detail::ObjectContainerT<ValueType> &t, const unsigned int)
{
    Pothos::serialization::base_object<Pothos::Detail::ObjectContainer>(t);
    ar & t.value;
}

template<class Archive>
void save(Archive & ar, const Pothos::Object &t, const unsigned int)
{
    const bool is_null = t.null();
    ar << is_null;
    if (not is_null) ar << t._impl;
}

template<class Archive>
void load(Archive & ar, Pothos::Object &t, const unsigned int)
{
    assert(t.null());
    bool is_null = false;
    ar >> is_null;
    if (not is_null) ar >> t._impl;
}

} // namespace serialization
} // namespace Pothos

POTHOS_SERIALIZATION_SPLIT_FREE(Pothos::Object)

#include <Pothos/serialization/complex.hpp>
#include <Pothos/serialization/string.hpp>
#include <Pothos/serialization/vector.hpp>
#include <Pothos/serialization/set.hpp>
#include <Pothos/serialization/map.hpp>
