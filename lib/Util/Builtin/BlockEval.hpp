// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include "EvalEnvironment.hpp"
#include <Pothos/Proxy.hpp>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <memory>
#include <string>
#include <vector>
#include <map>

class ProxyBlockEval
{
public:
    ProxyBlockEval(const std::shared_ptr<EvalEnvironment> &env):
        _evalEnv(env)
    {
        return;
    }

    void applyConstant(const std::string &name, const std::string &expr)
    {
        _evalEnv->registerConstantExpr(name, expr);
    }

    void removeConstant(const std::string &name)
    {
        _evalEnv->unregisterConstant(name);
    }

    Pothos::Object evalProperty(const std::string &key, const std::string &expr)
    {
        auto val = _evalEnv->eval(expr);
        _properties[key] = val;
        return val;
    }

    void setProperty(const std::string &key, const Pothos::Object &val)
    {
        _properties[key] = val;
    }

    void eval(const std::string &id, const Poco::JSON::Object::Ptr &blockDesc);

    void handleCall(const Poco::JSON::Object::Ptr &callObj);

    Pothos::Proxy getProxyBlock(void) const
    {
        return _proxyBlock;
    }

private:

    Pothos::Object lookupOrEvalAsType(const Poco::Dynamic::Var &arg);

    std::map<std::string, Pothos::Object> _properties;
    Pothos::Proxy _proxyBlock;
    std::shared_ptr<EvalEnvironment> _evalEnv;
};
