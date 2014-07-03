// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include "EvalEnvironment.hpp"
#include <Pothos/Proxy.hpp>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <string>
#include <vector>
#include <map>

class BlockEval
{
public:
    BlockEval(EvalEnvironment &env):
        evalEnv(env)
    {
        return;
    };

    Pothos::Object evalProperty(const std::string &key, const std::string &expr)
    {
        auto val = evalEnv.eval(expr);
        properties[key] = val;
        return val;
    }

    void eval(const std::string &id, const Poco::JSON::Object::Ptr &blockDesc);

    Poco::JSON::Object::Ptr inspect(void);

    Pothos::Proxy getProxyBlock(void) const
    {
        return proxyBlock;
    }

private:
    std::map<std::string, Pothos::Object> properties;
    Pothos::Proxy proxyBlock;
    EvalEnvironment &evalEnv;
};
