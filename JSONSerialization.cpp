// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Object/Serialize.hpp>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <sstream>

namespace Pothos { namespace serialization {
template<class Archive>
void save(Archive & ar, const Poco::JSON::Array::Ptr &t, const unsigned int)
{
    bool isNull = t.isNull();
    ar << isNull;
    if (isNull) return;
    std::ostringstream oss; t->stringify(oss);
    std::string s = oss.str(); ar << s;
}

template<class Archive>
void load(Archive & ar, Poco::JSON::Array::Ptr &t, const unsigned int)
{
    bool isNull = false;
    ar >> isNull;
    if (isNull) return;
    std::string s; ar >> s;
    Poco::JSON::Parser p; p.parse(s);
    t = p.getHandler()->asVar().extract<Poco::JSON::Array::Ptr>();
}

template<class Archive>
void save(Archive & ar, const Poco::JSON::Object::Ptr &t, const unsigned int)
{
    bool isNull = t.isNull();
    ar << isNull;
    if (isNull) return;
    std::ostringstream oss; t->stringify(oss);
    std::string s = oss.str(); ar << s;
}

template<class Archive>
void load(Archive & ar, Poco::JSON::Object::Ptr &t, const unsigned int)
{
    bool isNull = false;
    ar >> isNull;
    if (isNull) return;
    std::string s; ar >> s;
    Poco::JSON::Parser p; p.parse(s);
    t = p.getHandler()->asVar().extract<Poco::JSON::Object::Ptr>();
}
}}

POTHOS_SERIALIZATION_SPLIT_FREE(Poco::JSON::Array::Ptr)
POTHOS_OBJECT_SERIALIZE(Poco::JSON::Array::Ptr)
POTHOS_SERIALIZATION_SPLIT_FREE(Poco::JSON::Object::Ptr)
POTHOS_OBJECT_SERIALIZE(Poco::JSON::Object::Ptr)
