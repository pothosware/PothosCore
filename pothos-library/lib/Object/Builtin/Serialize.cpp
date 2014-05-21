// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Poco/Types.h>
#include <Pothos/Object/Serialize.hpp>
#include <Pothos/Object/Containers.hpp>

POTHOS_OBJECT_SERIALIZE(bool)
POTHOS_OBJECT_SERIALIZE(char)
POTHOS_OBJECT_SERIALIZE(signed char)
POTHOS_OBJECT_SERIALIZE(unsigned char)
POTHOS_OBJECT_SERIALIZE(signed short)
POTHOS_OBJECT_SERIALIZE(unsigned short)
POTHOS_OBJECT_SERIALIZE(signed int)
POTHOS_OBJECT_SERIALIZE(unsigned int)
POTHOS_OBJECT_SERIALIZE(signed long)
POTHOS_OBJECT_SERIALIZE(unsigned long)
POTHOS_OBJECT_SERIALIZE(signed long long)
POTHOS_OBJECT_SERIALIZE(unsigned long long)
POTHOS_OBJECT_SERIALIZE(float)
POTHOS_OBJECT_SERIALIZE(double)
POTHOS_OBJECT_SERIALIZE(std::complex<float>)
POTHOS_OBJECT_SERIALIZE(std::complex<double>)
POTHOS_OBJECT_SERIALIZE(std::string)
POTHOS_OBJECT_SERIALIZE(std::vector<std::string>)
POTHOS_OBJECT_SERIALIZE(Pothos::Object)
POTHOS_OBJECT_SERIALIZE(Pothos::ObjectVector)
POTHOS_OBJECT_SERIALIZE(Pothos::ObjectSet)
POTHOS_OBJECT_SERIALIZE(Pothos::ObjectMap)
POTHOS_OBJECT_SERIALIZE(Pothos::ObjectKwargs)
