// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/DType.hpp>
#include <Pothos/Framework/Exception.hpp>
#include <Pothos/Util/TypeInfo.hpp>
#include <Poco/SingletonHolder.h>
#include <Poco/StringTokenizer.h>
#include <Poco/RegularExpression.h>
#include <complex>
#include <map>

/***********************************************************************
 * officially supported element types
 **********************************************************************/
enum ElementTypes
{
    CustomType,
    Int8,
    UInt8,
    Int16,
    UInt16,
    Int32,
    UInt32,
    Int64,
    UInt64,
    ComplexInt8,
    ComplexUInt8,
    ComplexInt16,
    ComplexUInt16,
    ComplexInt32,
    ComplexUInt32,
    ComplexInt64,
    ComplexUInt64,
    Float32,
    Float64,
    ComplexFloat32,
    ComplexFloat64,
    MaximumType
};

/***********************************************************************
 * map typeids to an element type
 **********************************************************************/
class ElementTypeSuperMap
{
public:
    ElementTypeSuperMap(void)
    {
        #define loadNumericType(Type, Code, Name) \
            this->loadType<Type>(Code, Name); \
            this->loadType<std::complex<Type>>(Complex ## Code, "complex_" Name)
        #define loadIntegerType(Type, Code, Name) \
            loadNumericType(signed Type, Code, Name); \
            loadNumericType(unsigned Type, U ## Code, "u" Name)
        loadNumericType(char, Int8, "int8");
        loadIntegerType(char, Int8, "int8");
        loadIntegerType(short, Int16, "int16");
        loadIntegerType(int, Int32, "int32");
        #ifndef POCO_LONG_IS_64_BIT
        loadIntegerType(long, Int32, "int32");
        #else
        loadIntegerType(long, Int64, "int64");
        #endif
        loadIntegerType(long long, Int64, "int64");
        loadNumericType(float, Float32, "float32");
        loadNumericType(double, Float64, "float64");

        _elemTypeToElemSize[CustomType] = 0;
        _elemTypeToElemName[CustomType] = "custom";
    }

    size_t lookupElemType(const std::type_info &type)
    {
        try
        {
            return _typeHashToElemType.at(type.hash_code());
        }
        catch (const std::exception &ex)
        {
            throw Pothos::DTypeUnknownError("Pothos::DType("+Pothos::Util::typeInfoToString(type)+")", "unsupported element type");
        }
    }

    size_t lookupElemSize(const size_t type)
    {
        return _elemTypeToElemSize.at(type);
    }

    const std::string &lookupElemName(const size_t type)
    {
        return _elemTypeToElemName.at(type);
    }

private:
    template <typename Type>
    void loadType(const size_t elemType, const std::string &name)
    {
        _typeHashToElemType[typeid(Type).hash_code()] = elemType;
        _elemTypeToElemSize[elemType] = sizeof(Type);
        _elemTypeToElemName[elemType] = name;
    }

    std::map<size_t, size_t> _typeHashToElemType;
    std::map<size_t, size_t> _elemTypeToElemSize;
    std::map<size_t, std::string> _elemTypeToElemName;
};

static ElementTypeSuperMap &getElementTypeSuperMap(void)
{
    static Poco::SingletonHolder<ElementTypeSuperMap> sh;
    return *sh.get();
}

/***********************************************************************
 * Parse a name with markup
 **********************************************************************/
static void parseMarkupName(const std::string &markup, size_t &elemType, size_t &dimension)
{
    /*
    elemType = 0;
    dimension = 1;
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
            dimension *= size_t(std::stoull(tok));
        }
    }
    * 
    */
}

/***********************************************************************
 * DType implementation
 **********************************************************************/
Pothos::DType::DType(const char *name):
    _elemType(0), _elemSize(0), _dimension(0)
{
    parseMarkupName(name, _elemType, _dimension);
    _elemSize = getElementTypeSuperMap().lookupElemSize(_elemType);
}

Pothos::DType::DType(const std::string &name):
    _elemType(0), _elemSize(0), _dimension(0)
{
    parseMarkupName(name, _elemType, _dimension);
    _elemSize = getElementTypeSuperMap().lookupElemSize(_elemType);
}

Pothos::DType::DType(const std::string &name, const size_t dimension):
    _elemType(0), _elemSize(0), _dimension(dimension)
{
    parseMarkupName(name, _elemType, _dimension);
    _elemSize = getElementTypeSuperMap().lookupElemSize(_elemType);
}

Pothos::DType::DType(const std::type_info &type, const size_t dimension):
    _elemType(getElementTypeSuperMap().lookupElemType(type)),
    _elemSize(getElementTypeSuperMap().lookupElemSize(_elemType)),
    _dimension(dimension)
{
    return;
}

const std::string &Pothos::DType::name(void) const
{
   return getElementTypeSuperMap().lookupElemName(_elemType);
}

std::string Pothos::DType::toString(void) const
{
    std::string out = this->name();
    if (this->dimension() != 1)
    {
        out += " [" + std::to_string(this->dimension()) + "]";
    }
    return out;
}

#include <Pothos/Managed.hpp>

static auto managedDtype = Pothos::ManagedClass()
    .registerConstructor<Pothos::DType>()
    .registerConstructor<Pothos::DType, const std::string &>()
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::DType, name))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::DType, elemType))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::DType, elemSize))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::DType, dimension))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::DType, size))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::DType, toString))
    .registerStaticMethod<const Pothos::DType &, const Pothos::DType &>("equals", &Pothos::operator==)
    .commit("Pothos/DType");

#include <Pothos/Object/Serialize.hpp>

template<class Archive>
void Pothos::DType::serialize(Archive & ar, const unsigned int)
{
    ar & this->_elemType;
    ar & this->_elemSize;
    ar & this->_dimension;
}

template void Pothos::DType::serialize<Pothos::archive::polymorphic_iarchive>(Pothos::archive::polymorphic_iarchive &, const unsigned int);
template void Pothos::DType::serialize<Pothos::archive::polymorphic_oarchive>(Pothos::archive::polymorphic_oarchive &, const unsigned int);

POTHOS_OBJECT_SERIALIZE(Pothos::DType)
