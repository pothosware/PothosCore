// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/DType.hpp>
#include <Pothos/Framework/Exception.hpp>
#include <Pothos/Util/TypeInfo.hpp>
#include <Poco/Types.h>
#include <Poco/HashMap.h>
#include <Poco/SingletonHolder.h>
#include <Poco/NumberFormatter.h>
#include <Poco/NumberParser.h>
#include <Poco/StringTokenizer.h>
#include <Poco/RegularExpression.h>
#include <complex>

/***********************************************************************
 * Lookup table for name alias mapping
 **********************************************************************/
struct DTypeAliasLookup : Poco::HashMap<std::string, std::string>
{
    DTypeAliasLookup(void)
    {
        (*this)["byte"] = "int8";
        (*this)["octet"] = "int8";

        loadPrimitiveIntegers("char", "8");
        loadPrimitiveIntegers("short", "16");
        loadPrimitiveIntegers("int", "32");
        #ifndef POCO_LONG_IS_64_BIT
        loadPrimitiveIntegers("long", "32");
        #else
        loadPrimitiveIntegers("long", "64");
        #endif
        loadPrimitiveIntegers("long long", "64");
        loadPrimitiveIntegers("longlong", "64"); //with and without spaces

        (*this)["float"] = "float32";
        (*this)["double"] = "float64";

        (*this)["std::complex<float>"] = "complex64";
        (*this)["std::complex<double>"] = "complex128";

        #define makeTypeIdEntry(type) \
            (*this)[Pothos::Util::typeInfoToString(typeid(type))] = (*this)[#type]
        makeTypeIdEntry(char);
        makeTypeIdEntry(signed char);
        makeTypeIdEntry(unsigned char);
        makeTypeIdEntry(signed short);
        makeTypeIdEntry(unsigned short);
        makeTypeIdEntry(signed int);
        makeTypeIdEntry(unsigned int);
        makeTypeIdEntry(signed long);
        makeTypeIdEntry(unsigned long);
        makeTypeIdEntry(signed long long);
        makeTypeIdEntry(unsigned long long);
        makeTypeIdEntry(float);
        makeTypeIdEntry(double);
        makeTypeIdEntry(std::complex<float>);
        makeTypeIdEntry(std::complex<double>);
    }

    void loadPrimitiveIntegers(const std::string &prim, const std::string &size)
    {
        (*this)["int"+size] = "int"+size;
        (*this)["sint"+size] = "int"+size;
        (*this)["uint"+size] = "uint"+size;
        (*this)["int"+size+"_t"] = "int"+size;
        (*this)["sint"+size+"_t"] = "int"+size;
        (*this)["uint"+size+"_t"] = "uint"+size;
        (*this)[prim] = "int"+size;
        (*this)["u"+prim] = "uint"+size;
        (*this)["s"+prim] = "int"+size;
        (*this)["unsigned "+prim] = "uint"+size;
        (*this)["signed "+prim] = "int"+size;
    }

    std::string lookup(const std::string &name) const
    {
        const auto it = this->find(name);
        if (it == this->end()) return name;
        return it->second;
    }
};

static DTypeAliasLookup &getDTypeAliasLookup(void)
{
    static Poco::SingletonHolder<DTypeAliasLookup> sh;
    return *sh.get();
}

/***********************************************************************
 * Lookup table for name to size mapping
 **********************************************************************/
struct DTypeSizeLookup : Poco::HashMap<std::string, size_t>
{
    DTypeSizeLookup(void)
    {
        (*this)["bool"] = sizeof(bool);
        (*this)["int8"] = sizeof(Poco::Int8);
        (*this)["uint8"] = sizeof(Poco::UInt8);
        (*this)["int16"] = sizeof(Poco::Int16);
        (*this)["uint16"] = sizeof(Poco::UInt16);
        (*this)["int32"] = sizeof(Poco::Int32);
        (*this)["uint32"] = sizeof(Poco::UInt32);
        (*this)["int64"] = sizeof(Poco::Int64);
        (*this)["uint64"] = sizeof(Poco::UInt64);
        (*this)["float32"] = sizeof(float);
        (*this)["float64"] = sizeof(double);
        (*this)["complex64"] = sizeof(std::complex<float>);
        (*this)["complex128"] = sizeof(std::complex<double>);
    }

    size_t lookup(const std::string &name) const
    {
        const auto it = this->find(name);
        if (it == this->end()) throw Pothos::DTypeUnknownError(
            "Pothos::DType("+name+")", "unknown name");
        return it->second;
    }
};

static DTypeSizeLookup &getDTypeSizeLookup(void)
{
    static Poco::SingletonHolder<DTypeSizeLookup> sh;
    return *sh.get();
}

/***********************************************************************
 * Parse a name with markup
 **********************************************************************/
static void parseMarkupName(const std::string &markup, std::string &name, Pothos::DType::Shape &shape)
{
    shape.clear();
    size_t count = 0;
    for (const auto &tok : Poco::StringTokenizer(
        markup, ",", Poco::StringTokenizer::TOK_TRIM | Poco::StringTokenizer::TOK_IGNORE_EMPTY))
    {
        if (count++ == 0)
        {
            name = getDTypeAliasLookup().lookup(tok);
        }
        else
        {
            if (not Poco::RegularExpression("^\\d+$").match(tok, 0))
            {
                throw Pothos::DTypeUnknownError("Pothos::DType("+markup+")", "bad markup format");
            }
            shape.push_back(size_t(Poco::NumberParser::parseUnsigned(tok)));
        }
    }
}

/***********************************************************************
 * DType implementation
 **********************************************************************/
Pothos::DType::DType(void):
    _elemSize(0), _size(0)
{
    return;
}

Pothos::DType::DType(const char *name):
    _elemSize(0), _size(0)
{
    parseMarkupName(name, _name, _shape); //also sets alias
    _elemSize = getDTypeSizeLookup().lookup(_name); //uses alias for lookup
    _size = _elemSize;
}

Pothos::DType::DType(const std::string &name):
    _elemSize(0), _size(0)
{
    parseMarkupName(name, _name, _shape); //also sets alias
    _elemSize = getDTypeSizeLookup().lookup(_name); //uses alias for lookup
    _size = _elemSize;
}

Pothos::DType::DType(const std::string &name, const Shape &shape):
    _name(getDTypeAliasLookup().lookup(name)),
    _shape(shape),
    _elemSize(getDTypeSizeLookup().lookup(_name)), //uses alias for lookup
    _size(_elemSize)
{
    for (size_t i = 0; i < shape.size(); i++) _size *= shape[i];
}

Pothos::DType::DType(const std::string &name, const size_t elemSize, const Shape &shape):
    _name(name),
    _shape(shape),
    _elemSize(elemSize),
    _size(_elemSize)
{
    for (size_t i = 0; i < shape.size(); i++) _size *= shape[i];
}

Pothos::DType::DType(const std::type_info &type, const Shape &shape):
    _name(getDTypeAliasLookup().lookup(Pothos::Util::typeInfoToString(type))),
    _shape(shape),
    _elemSize(getDTypeSizeLookup().lookup(_name)), //uses alias for lookup
    _size(_elemSize)
{
    for (size_t i = 0; i < shape.size(); i++) _size *= shape[i];
}

std::string Pothos::DType::toString(void) const
{
    std::string out = this->name();
    if (not this->shape().empty())
    {
        out += " [";
        for (size_t i = 0; i < this->shape().size(); i++)
        {
            out += Poco::NumberFormatter::format(this->shape()[i]);
            if (i+1 != this->shape().size()) out += " x ";
        }
        out += "]";
    }
    return out;
}

bool Pothos::operator==(const DType &lhs, const DType &rhs)
{
    return (lhs.name() == rhs.name())
        and (lhs.shape() == rhs.shape())
        and (lhs.elemSize() == rhs.elemSize());
}

#include <Pothos/Managed.hpp>

static auto managedDtype = Pothos::ManagedClass()
    .registerConstructor<Pothos::DType>()
    .registerConstructor<Pothos::DType, const std::string &>()
    .registerConstructor<Pothos::DType, const std::string &, const Pothos::DType::Shape &>()
    .registerConstructor<Pothos::DType, const std::string &, const size_t, const Pothos::DType::Shape &>()
    //.registerConstructor<Pothos::DType, const std::type_info &, const Pothos::DType::Shape &>()
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::DType, name))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::DType, shape))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::DType, elemSize))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::DType, size))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::DType, toString))
    .registerStaticMethod<const Pothos::DType &, const Pothos::DType &>("equals", &Pothos::operator==)
    .commit("Pothos/DType");

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
POTHOS_OBJECT_SERIALIZE(Pothos::DType)
