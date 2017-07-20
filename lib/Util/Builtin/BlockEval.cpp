// Copyright (c) 2014-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "BlockEval.hpp"

ProxyBlockEval::ProxyBlockEval(const std::string &path, const std::shared_ptr<Pothos::Util::EvalEnvironment> &evalEnv):
    _path(path),
    _evalEnv(evalEnv)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto proxy = env->findProxy("Pothos/Util/DocUtils");
    _blockDesc = json::parse(proxy.call<std::string>("dumpJsonAt", path));
}

void ProxyBlockEval::eval(const std::string &id)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");
    _proxyBlock = Pothos::Proxy(); //release old handle

    //load up the constructor args
    std::vector<Pothos::Proxy> ctorArgs;
    for (const auto &arg : _blockDesc["args"])
    {
        const auto obj = this->lookupOrEvalAsType(arg);
        ctorArgs.push_back(env->convertObjectToProxy(obj));
    }

    //create the block
    try
    {
        _proxyBlock = registry.getHandle()->call(_path, ctorArgs.data(), ctorArgs.size());
    }
    catch (const Pothos::Exception &ex)
    {
        throw Pothos::Exception("ProxyBlockEval factory("+_path+")", ex);
    }
    _proxyBlock.callVoid("setName", id);

    //make the calls
    if (_blockDesc.count("calls"))
    {
        for (const auto &call : _blockDesc["calls"])
        {
            this->_handleCall(call);
        }
    }
}

void ProxyBlockEval::handleCall(const std::string &callName)
{
    if (_blockDesc.count("calls") == 0) return;
    for (const auto &call : _blockDesc["calls"])
    {
        if (call["name"].get<std::string>() == callName)
        {
            this->_handleCall(call);
        }
    }
}

void ProxyBlockEval::_handleCall(const json &callObj)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    const auto callName = callObj["name"].get<std::string>();
    std::vector<Pothos::Proxy> callArgs;
    if (callObj.count("args")) for (const auto &arg : callObj["args"])
    {
        const auto obj = this->lookupOrEvalAsType(arg);
        callArgs.push_back(env->convertObjectToProxy(obj));
    }
    try
    {
        _proxyBlock.getHandle()->call(callName, callArgs.data(), callArgs.size());
    }
    catch (const Pothos::Exception &ex)
    {
        throw Pothos::Exception("ProxyBlockEval call("+callName+")", ex);
    }
}

Pothos::Object ProxyBlockEval::lookupOrEvalAsType(const json &arg)
{
    //this is not an expression, but a native JSON type
    if (not arg.is_string()) return _evalEnv->eval(arg.dump());

    //the expression is an already evaluated property
    const auto expr = arg.get<std::string>();
    if (_properties.count(expr) != 0) return _properties.at(expr);

    //otherwise the expression must be evaluated
    //with the evaluated properties as global variables
    //use a new eval to avoid poisoning the globals
    Pothos::Util::EvalEnvironment evalEnv;
    for (const auto &pair : _properties)
    {
        //Register can fail for non-primitive types
        //but those are not used in expressions anyway.
        try {evalEnv.registerConstantObj(pair.first, pair.second);}
        catch (...){}
    }
    return evalEnv.eval(expr);
}

#include <Pothos/Managed.hpp>

static auto managedProxyBlockEval = Pothos::ManagedClass()
    .registerConstructor<ProxyBlockEval, const std::string &, const std::shared_ptr<Pothos::Util::EvalEnvironment> &>()
    .registerMethod(POTHOS_FCN_TUPLE(ProxyBlockEval, applyConstant))
    .registerMethod(POTHOS_FCN_TUPLE(ProxyBlockEval, removeConstant))
    .registerMethod(POTHOS_FCN_TUPLE(ProxyBlockEval, evalProperty))
    .registerMethod(POTHOS_FCN_TUPLE(ProxyBlockEval, setProperty))
    .registerMethod(POTHOS_FCN_TUPLE(ProxyBlockEval, eval))
    .registerMethod(POTHOS_FCN_TUPLE(ProxyBlockEval, handleCall))
    .registerMethod(POTHOS_FCN_TUPLE(ProxyBlockEval, getProxyBlock))
    .commit("Pothos/Util/BlockEval");
