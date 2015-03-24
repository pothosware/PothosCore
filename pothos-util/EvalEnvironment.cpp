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
#include <Poco/RWLock.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include "mpParser.h"

struct EvalEnvironment::Impl
{
    std::map<std::string, Pothos::Object> evalCache;
    std::map<std::string, std::string> errorCache;
    Poco::RWLock mutex;
};

EvalEnvironment::EvalEnvironment(void):
    _impl(new Impl())
{
    return;
}

Pothos::Object EvalEnvironment::eval(const std::string &expr_)
{
    const auto expr = Poco::trim(expr_);

    //check the cache
    {
        Poco::RWLock::ScopedReadLock l(_impl->mutex);
        auto it = _impl->evalCache.find(expr);
        if (it != _impl->evalCache.end()) return it->second;

        //errors cache
        auto errorIt = _impl->errorCache.find(expr);
        if (errorIt != _impl->errorCache.end()) throw Pothos::Exception("EvalEnvironment::eval("+expr+")", errorIt->second);
    }

    //try to perform the evaluation
    Pothos::Object result;
    try
    {
        result = this->evalNoCache(expr);
    }
    catch (const Pothos::Exception &ex)
    {
        //cache the error
        Poco::RWLock::ScopedWriteLock l(_impl->mutex);
        _impl->errorCache[expr] = ex.displayText();
        throw Pothos::Exception("EvalEnvironment::eval("+expr+")", ex.displayText());
    }

    //cache result and return
    {
        Poco::RWLock::ScopedWriteLock l(_impl->mutex);
        _impl->evalCache[expr] = result;
    }

    return result;
}

Pothos::Object EvalEnvironment::evalNoCache(const std::string &expr)
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

    mup::ParserX p(mup::pckALL_COMPLEX);
    p.DefineConst("True", true);
    p.DefineConst("False", false);
    p.DefineConst("j", std::complex<double>(0.0, 1.0));
    try
    {
        p.SetExpr(expr);
        mup::Value result = p.Eval();
        switch (result.GetType())
        {
        case 'b': return Pothos::Object(result.GetBool());
        case 'i': return Pothos::Object(result.GetInteger());
        case 'f': return Pothos::Object(result.GetFloat());
        case 'c': return Pothos::Object(result.GetComplex());
        case 's': return Pothos::Object(result.GetString());
        //TODO m for matrix type
        }
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
    .commit("Pothos/Util/EvalEnvironment");
