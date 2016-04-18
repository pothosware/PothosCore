// Copyright (c) 2014-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/DType.hpp>
#include <Pothos/Framework/Exception.hpp>
#include <Pothos/Util/TypeInfo.hpp>
#include <Poco/SingletonHolder.h>
#include <Poco/StringTokenizer.h>
#include <Poco/RegularExpression.h>
#include <Poco/HashMap.h>
#include <Poco/String.h>
#include <complex>
#include <map>

/***********************************************************************
 * officially supported element types
 **********************************************************************/
static const int Custom = (1 << 0);
static const int Signed = (1 << 1);
static const int Integer = (1 << 2);
static const int Float = (1 << 3);
static const int Complex = (1 << 4);
static const int Bytes1 = (0 << 5);
static const int Bytes2 = (1 << 5);
static const int Bytes4 = (2 << 5);
static const int Bytes8 = (3 << 5);
enum ElementTypes
{
    EmptyType = 0,
    CustomType = Custom,
    Int8 = Signed | Integer | Bytes1,
    UInt8 = Integer | Bytes1,
    Int16 = Signed | Integer | Bytes2,
    UInt16 = Integer | Bytes2,
    Int32 = Signed | Integer | Bytes4,
    UInt32 = Integer | Bytes4,
    Int64 = Signed | Integer | Bytes8,
    UInt64 = Integer | Bytes8,
    ComplexInt8 = Complex | Signed | Integer | Bytes1,
    ComplexUInt8 = Complex | Integer | Bytes1,
    ComplexInt16 = Complex | Signed | Integer | Bytes2,
    ComplexUInt16 = Complex | Integer | Bytes2,
    ComplexInt32 = Complex | Signed | Integer | Bytes4,
    ComplexUInt32 = Complex | Integer | Bytes4,
    ComplexInt64 = Complex | Signed | Integer | Bytes8,
    ComplexUInt64 = Complex | Integer | Bytes8,
    Float32 = Float | Bytes4,
    Float64 = Float | Bytes8,
    ComplexFloat32 = Complex | Float | Bytes4,
    ComplexFloat64 = Complex | Float | Bytes8,
};

/***********************************************************************
 * lookup map for various element type properties
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
            loadNumericType(signed Type, Code, "s" Name); \
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

        //special non-generated cases
        _aliasToElementType[""] = EmptyType;
        _aliasToElementType["unspecified"] = EmptyType;
        _elemTypeToElemSize[EmptyType] = 1;
        _elemTypeToElemName[EmptyType] = "unspecified";
        _aliasToElementType["custom"] = CustomType;
        _elemTypeToElemSize[CustomType] = 1;
        _elemTypeToElemName[CustomType] = "custom";
        _aliasToElementType["custom"] = CustomType;
        _aliasToElementType["byte"] = Int8;
        _aliasToElementType["octet"] = Int8;
        _aliasToElementType["float"] = Float32;
        _aliasToElementType["double"] = Float64;
        _aliasToElementType["complex64"] = ComplexFloat32;
        _aliasToElementType["complex128"] = ComplexFloat64;
        #define declareNativeAlias(Code, Name) \
            _aliasToElementType[Name] = Code; \
            _aliasToElementType["s" Name] = Code; \
            _aliasToElementType["u" Name] = U ## Code
        declareNativeAlias(Int8, "char");
        declareNativeAlias(Int16, "short");
        declareNativeAlias(Int32, "int");
        #ifndef POCO_LONG_IS_64_BIT
        declareNativeAlias(Int32, "long");
        #else
        declareNativeAlias(Int64, "long");
        #endif
        declareNativeAlias(Int64, "long long");
        declareNativeAlias(Int64, "longlong");
        declareNativeAlias(Int64, "llong");
    }

    size_t lookupAlias(const std::string &alias) const
    {
        auto it = _aliasToElementType.find(Poco::toLower(alias));
        if (it == _aliasToElementType.end())
        {
            throw Pothos::DTypeUnknownError("Pothos::DType("+alias+")", "unknown type alias");
        }
        return it->second;
    }

    size_t lookupElemType(const std::type_info &type) const
    {
        try
        {
            return _typeHashToElemType.at(type.hash_code());
        }
        catch (const std::exception &)
        {
            throw Pothos::DTypeUnknownError("Pothos::DType("+Pothos::Util::typeInfoToString(type)+")", "unsupported element type");
        }
    }

    size_t lookupElemSize(const size_t type) const
    {
        return _elemTypeToElemSize.at(type);
    }

    const std::string &lookupElemName(const size_t type) const
    {
        return _elemTypeToElemName.at(type);
    }

private:
    template <typename Type>
    void loadType(const size_t elemType, const std::string &name)
    {
        _aliasToElementType[name] = elemType;
        _aliasToElementType[typeid(Type).name()] = elemType;
        _aliasToElementType[Pothos::Util::typeInfoToString(typeid(Type))] = elemType;
        _typeHashToElemType[typeid(Type).hash_code()] = elemType;
        _elemTypeToElemSize[elemType] = sizeof(Type);
        _elemTypeToElemName[elemType] = name;
    }

    Poco::HashMap<std::string, size_t> _aliasToElementType;
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
static size_t parseMarkupName(const std::string &markup, size_t &dimension)
{
    const auto commaPos = markup.find(",");
    if (commaPos == std::string::npos) return getElementTypeSuperMap().lookupAlias(markup);

    const auto alias = Poco::trim(markup.substr(0, commaPos));
    const auto dimStr = Poco::trim(markup.substr(commaPos+1));

    try
    {
        dimension = size_t(std::stoull(dimStr));
    }
    catch (const std::exception &ex)
    {
        throw Pothos::DTypeUnknownError("Pothos::DType("+markup+")", "cant parse markup: " + std::string(ex.what()));
    }
    return getElementTypeSuperMap().lookupAlias(alias);
}

/***********************************************************************
 * DType implementation
 **********************************************************************/
Pothos::DType::DType(const char *markup):
    _elemType(0), _elemSize(0), _dimension(1)
{
    _elemType = parseMarkupName(markup, _dimension);
    _elemSize = getElementTypeSuperMap().lookupElemSize(_elemType);
}

Pothos::DType::DType(const std::string &markup):
    _elemType(0), _elemSize(0), _dimension(1)
{
    _elemType = parseMarkupName(markup, _dimension);
    _elemSize = getElementTypeSuperMap().lookupElemSize(_elemType);
}

Pothos::DType::DType(const std::string &alias, const size_t dimension):
    _elemType(0), _elemSize(0), _dimension(dimension)
{
    _elemType = getElementTypeSuperMap().lookupAlias(alias);
    _elemSize = getElementTypeSuperMap().lookupElemSize(_elemType);
}

Pothos::DType::DType(const std::type_info &type, const size_t dimension):
    _elemType(getElementTypeSuperMap().lookupElemType(type)),
    _elemSize(getElementTypeSuperMap().lookupElemSize(_elemType)),
    _dimension(dimension)
{
    return;
}

Pothos::DType Pothos::DType::fromDType(const DType &dtype, const size_t dimension)
{
    DType newDType = dtype;
    newDType._dimension = dimension;
    return newDType;
}

const std::string &Pothos::DType::name(void) const
{
   return getElementTypeSuperMap().lookupElemName(_elemType);
}

std::string Pothos::DType::toString(void) const
{
    std::string out = this->name();
    if (this->dimension() != 1 or this->isCustom())
    {
        out += "[" + std::to_string(this->dimension()) + "]";
    }
    return out;
}

std::string Pothos::DType::toMarkup(void) const
{
    std::string out = this->name();
    if (this->dimension() != 1 or this->isCustom())
    {
        out += ", " + std::to_string(this->dimension());
    }
    return out;
}

bool Pothos::DType::isCustom(void) const
{
    return (_elemType & Custom) != 0;
}

bool Pothos::DType::isFloat(void) const
{
    return (_elemType & Float) != 0;
}

bool Pothos::DType::isInteger(void) const
{
    return (_elemType & Integer) != 0;
}

bool Pothos::DType::isSigned(void) const
{
    return (_elemType & Signed) != 0;
}

bool Pothos::DType::isComplex(void) const
{
    return (_elemType & Complex) != 0;
}

#include <Pothos/Managed.hpp>

static auto managedDtype = Pothos::ManagedClass()
    .registerConstructor<Pothos::DType>()
    .registerConstructor<Pothos::DType, const std::string &>()
    .registerConstructor<Pothos::DType, const std::string &, const size_t>()
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::DType, name))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::DType, elemType))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::DType, elemSize))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::DType, dimension))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::DType, size))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::DType, toString))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::DType, isCustom))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::DType, isFloat))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::DType, isInteger))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::DType, isSigned))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::DType, isComplex))
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
