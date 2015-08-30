// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Poco/Types.h>
#include <Pothos/Object/Serialize.hpp>
#include <Pothos/Object/Containers.hpp>

#define declare_serialize_with_containers(type) \
    POTHOS_OBJECT_SERIALIZE(type) \
    POTHOS_OBJECT_SERIALIZE(std::vector<type>)

declare_serialize_with_containers(bool)
declare_serialize_with_containers(char)
declare_serialize_with_containers(signed char)
declare_serialize_with_containers(unsigned char)
declare_serialize_with_containers(signed short)
declare_serialize_with_containers(unsigned short)
declare_serialize_with_containers(signed int)
declare_serialize_with_containers(unsigned int)
declare_serialize_with_containers(signed long)
declare_serialize_with_containers(unsigned long)
declare_serialize_with_containers(signed long long)
declare_serialize_with_containers(unsigned long long)
declare_serialize_with_containers(float)
declare_serialize_with_containers(double)
declare_serialize_with_containers(std::complex<float>)
declare_serialize_with_containers(std::complex<double>)
declare_serialize_with_containers(std::string)
POTHOS_OBJECT_SERIALIZE(Pothos::Object)
POTHOS_OBJECT_SERIALIZE(Pothos::ObjectVector)
POTHOS_OBJECT_SERIALIZE(Pothos::ObjectSet)
POTHOS_OBJECT_SERIALIZE(Pothos::ObjectMap)
POTHOS_OBJECT_SERIALIZE(Pothos::ObjectKwargs)
