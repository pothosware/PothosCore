// Copyright (c) 2014-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Util/EvalEnvironment.hpp>
#include <Pothos/Object.hpp>
#include <Pothos/Framework/DType.hpp>
#include <Pothos/Object/Containers.hpp>
#include <Pothos/Proxy.hpp>
#include <Poco/String.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <mutex>
#include "mpParser.h"

static const std::string mapTypeId("__map__");
static const std::string tmpTypeId("__tmp__");

/***********************************************************************
 * convert parser value into a native object
 **********************************************************************/
static Pothos::Object mupValueToObject(mup::IValue &val)
{
    switch (val.GetType())
    {
    case 'b': return Pothos::Object(val.GetBool());
    case 'i': return Pothos::Object(val.GetInteger());
    case 'f': return Pothos::Object(val.GetFloat());
    case 'c': return Pothos::Object(val.GetComplex());
    case 's': return Pothos::Object(val.GetString());
    case 'm': break;
    default: Pothos::Exception("EvalEnvironment::mupValueToObject()", "unknown type " + val.AsciiDump());
    }

    assert(val.GetType() == 'm');
    auto env = Pothos::ProxyEnvironment::make("managed");

    //detect if this array is a flattened map
    const bool isMap = (val.GetCols() % 2) == 1 and
        val.At(0, 0).GetType() == 's' and
        val.At(0, 0).GetString() == mapTypeId;

    //support array to vector
    Pothos::ProxyVector vec(val.GetCols());
    for (size_t i = 0; i < vec.size(); i++)
    {
        const auto obj_i = mupValueToObject(val.At(0, i));
        vec[i] = env->convertObjectToProxy(obj_i);
    }
    if (not isMap) return Pothos::Object(vec);

    //special case map mode (array -> vector -> map)
    Pothos::ProxyMap map;
    for (size_t i = 0; i < vec.size()/2; i++)
    {
        map[vec[i*2 + 1]] = vec[i*2 + 2];
    }
    return Pothos::Object(map);
}

/***********************************************************************
 * convert native object into a parser value
 **********************************************************************/
static mup::Value objectToMupValue(const Pothos::Object &obj)
{
    //types that the parser library specifically typedefs
    if (obj.type() == typeid(mup::string_type)) return mup::Value(obj.extract<mup::string_type>());
    if (obj.type() == typeid(mup::float_type)) return mup::Value(obj.extract<mup::float_type>());
    if (obj.type() == typeid(mup::bool_type)) return mup::Value(obj.extract<mup::bool_type>());
    if (obj.type() == typeid(mup::int_type)) return mup::Value(obj.extract<mup::int_type>());
    if (obj.type() == typeid(mup::cmplx_type)) return mup::Value(obj.extract<mup::cmplx_type>());

    //other numeric types
    try
    {
        Pothos::DType dtype(obj.type());
        if (dtype.isComplex()) return mup::Value(obj.operator mup::cmplx_type());
        if (dtype.isFloat()) return mup::Value(obj.operator mup::float_type());
        //types that fit into the parser's integer type, otherwise use floating point
        if (dtype.size() <= sizeof(mup::int_type)) return mup::Value(obj.operator mup::int_type());
        else return mup::Value(obj.operator mup::float_type());
    }
    catch (...) {}

    //support proxy vector to parser array
    if (obj.canConvert(typeid(Pothos::ProxyVector)))
    {
        const Pothos::ProxyVector vec = obj;
        mup::Value arr(1, vec.size(), 0.0);
        for (size_t i = 0; i < vec.size(); i++)
        {
            arr.At(0, i) = objectToMupValue(vec[i].toObject());
        }
        return arr;
    }

    //support proxy map to parser array
    if (obj.canConvert(typeid(Pothos::ProxyMap)))
    {
        const Pothos::ProxyMap map = obj;
        mup::Value arr(1, map.size()*2+1, 0.0);
        size_t i = 0;
        arr.At(0, i++) = mup::Value(mapTypeId);
        for (const auto &pair : map)
        {
            arr.At(0, i++) = objectToMupValue(pair.first.toObject());
            arr.At(0, i++) = objectToMupValue(pair.second.toObject());
        }
        return arr;
    }

    throw Pothos::Exception("EvalEnvironment::objectToMupValue()", "unknown type " + obj.getTypeString());
}

/***********************************************************************
 * Evaluator implementation
 **********************************************************************/
struct Pothos::Util::EvalEnvironment::Impl
{
    Impl(void):
        p(mup::pckALL_COMPLEX)
    {
        p.DefineConst("True", true);
        p.DefineConst("False", false);
        p.DefineConst("j", std::complex<double>(0.0, 1.0));
    }
    std::mutex parserMutex;
    mup::ParserX p;
};

std::shared_ptr<Pothos::Util::EvalEnvironment> Pothos::Util::EvalEnvironment::make(void)
{
    return std::shared_ptr<EvalEnvironment>(new EvalEnvironment());
}

Pothos::Util::EvalEnvironment::EvalEnvironment(void):
    _impl(new Impl())
{
    return;
}

void Pothos::Util::EvalEnvironment::registerConstantExpr(const std::string &key, const std::string &expr)
{
    try
    {
        const auto result = objectToMupValue(this->eval(expr));
        this->unregisterConstant(key);
        _impl->p.DefineConst(key, result);
    }
    catch (const mup::ParserError &ex)
    {
        throw Pothos::Exception("EvalEnvironment::eval("+expr+")", ex.GetMsg());
    }
}

void Pothos::Util::EvalEnvironment::registerConstantObj(const std::string &key, const Pothos::Object &obj)
{
    try
    {
        const auto result = objectToMupValue(obj);
        this->unregisterConstant(key);
        _impl->p.DefineConst(key, result);
    }
    catch (const mup::ParserError &ex)
    {
        throw Pothos::Exception("EvalEnvironment::registerConstantObj("+key+")", ex.GetMsg());
    }
}

void Pothos::Util::EvalEnvironment::unregisterConstant(const std::string &key)
{
    if (_impl->p.IsConstDefined(key)) _impl->p.RemoveConst(key);
}

Pothos::Object Pothos::Util::EvalEnvironment::eval(const std::string &expr)
{
    if (Poco::trim(expr).empty()) throw Pothos::Exception("EvalEnvironment::eval()", "expression is empty");

    //handle multiple containers in top level
    const auto tokens = EvalEnvironment::splitExpr(expr);
    if (tokens.size() > 1)
    {
        size_t index = 0;
        std::string newExpr;
        for (const auto &tok : tokens)
        {
            if (tok.empty()) continue;
            if (tok.front() == '[' or tok.front() == '{')
            {
                const std::string key = tmpTypeId + std::to_string(index++);
                this->registerConstantObj(key, this->eval(tok));
                newExpr += key;
            }
            else
            {
                newExpr += tok;
            }
        }
        return this->eval(newExpr);
    }

    //list syntax mode
    const auto inBrackets = expr.size() >= 2 and expr.front() == '[' and expr.back() == ']';
    if (inBrackets) return this->_evalList(expr);

    //map syntax mode
    const auto inBraces = expr.size() >= 2 and expr.front() == '{' and expr.back() == '}';
    if (inBraces) return this->_evalMap(expr);

    //use the muparser
    try
    {
        std::lock_guard<std::mutex> lock(_impl->parserMutex);
        _impl->p.SetExpr(expr);
        mup::Value result = _impl->p.Eval();
        return mupValueToObject(result);
    }
    catch (const mup::ParserError &ex)
    {
        throw Pothos::Exception("EvalEnvironment::eval("+expr+")", ex.GetMsg());
    }

    throw Pothos::Exception("EvalEnvironment::eval("+expr+")", "unknown result");
}

Pothos::Object Pothos::Util::EvalEnvironment::_evalList(const std::string &expr)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    Pothos::ProxyVector vec;
    const auto noBrackets = expr.substr(1, expr.size()-2);
    for (const auto &tok : EvalEnvironment::splitExpr(noBrackets, ','))
    {
        try
        {
            vec.emplace_back(env->convertObjectToProxy(this->eval(tok)));
        }
        catch (const Pothos::Exception &ex)
        {
            throw Pothos::Exception("EvalEnvironment::eval("+expr+")", ex.message());
        }
    }
    return Pothos::Object(vec);
}

Pothos::Object Pothos::Util::EvalEnvironment::_evalMap(const std::string &expr)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    Pothos::ProxyMap map;
    const auto noBrackets = expr.substr(1, expr.size()-2);
    for (const auto &tok : EvalEnvironment::splitExpr(noBrackets, ','))
    {
        const auto keyVal = EvalEnvironment::splitExpr(tok, ':');
        if (keyVal.size() != 2) throw Pothos::Exception("EvalEnvironment::eval("+tok+")", "not key:value");
        try
        {
            const auto key = env->convertObjectToProxy(this->eval(keyVal[0]));
            const auto val = env->convertObjectToProxy(this->eval(keyVal[1]));
            map.emplace(key, val);
        }
        catch (const Pothos::Exception &ex)
        {
            throw Pothos::Exception("EvalEnvironment::eval("+expr+")", ex.message());
        }
    }
    return Pothos::Object(map);
}

#include <Pothos/Managed.hpp>

static auto managedEvalEnvironment = Pothos::ManagedClass()
    .registerConstructor<Pothos::Util::EvalEnvironment>()
    .registerStaticMethod(POTHOS_FCN_TUPLE(Pothos::Util::EvalEnvironment, make))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Util::EvalEnvironment, eval))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Util::EvalEnvironment, registerConstantExpr))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Util::EvalEnvironment, registerConstantObj))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Util::EvalEnvironment, unregisterConstant))
    .commit("Pothos/Util/EvalEnvironment");
