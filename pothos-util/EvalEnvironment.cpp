// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "EvalEnvironment.hpp"
#include <Pothos/Util/Compiler.hpp>
#include <Pothos/Util/EvalInterface.hpp>
#include <Pothos/Object.hpp>
#include <Pothos/Object/Containers.hpp>
#include <Pothos/Proxy.hpp>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <mutex>
#include "mpParser.h"

static Pothos::Object mupValueToObject(const mup::Value &val)
{
    switch (val.GetType())
    {
    case 'b': return Pothos::Object(val.GetBool());
    case 'i': return Pothos::Object(val.GetInteger());
    case 'f': return Pothos::Object(val.GetFloat());
    case 'c': return Pothos::Object(val.GetComplex());
    case 's': return Pothos::Object(val.GetString());
    //TODO m for matrix type
    }
    return Pothos::Object();
}

struct EvalEnvironment::Impl
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

EvalEnvironment::EvalEnvironment(void):
    _impl(new Impl())
{
    return;
}

void EvalEnvironment::registerConstant(const std::string &key, const std::string &expr)
{
    try
    {
        std::lock_guard<std::mutex> lock(_impl->parserMutex);
        _impl->p.SetExpr(expr);
        mup::Value result = _impl->p.Eval();

        if (_impl->p.IsConstDefined(key)) _impl->p.RemoveConst(key);
        _impl->p.DefineConst(key, result);
    }
    catch (const mup::ParserError &ex)
    {
        throw Pothos::Exception("EvalEnvironment::eval("+expr+")", ex.GetMsg());
    }
}

Pothos::Object EvalEnvironment::eval(const std::string &expr)
{
    if (expr.empty()) throw Pothos::Exception("EvalEnvironment::eval()", "expression is empty");
    const auto inBrackets = expr.size() >= 2 and expr.front() == '[' and expr.back() == ']';
    const auto inBraces = expr.size() >= 2 and expr.front() == '{' and expr.back() == '}';

    //list syntax mode
    if (inBrackets)
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

    //map syntax mode
    if (inBraces)
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

#include <Pothos/Managed.hpp>

static auto managedEvalEnvironment = Pothos::ManagedClass()
    .registerConstructor<EvalEnvironment>()
    .registerStaticMethod(POTHOS_FCN_TUPLE(EvalEnvironment, make))
    .registerMethod(POTHOS_FCN_TUPLE(EvalEnvironment, eval))
    .registerMethod(POTHOS_FCN_TUPLE(EvalEnvironment, registerConstant))
    .commit("Pothos/Util/EvalEnvironment");
