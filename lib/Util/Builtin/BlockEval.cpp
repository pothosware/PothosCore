// Copyright (c) 2014-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "BlockEval.hpp"
#include <Poco/JSON/Parser.h>

ProxyBlockEval::ProxyBlockEval(const std::string &path, const std::shared_ptr<Pothos::Util::EvalEnvironment> &evalEnv):
    _path(path),
    _evalEnv(evalEnv)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto proxy = env->findProxy("Pothos/Util/DocUtils");
    const auto json = proxy.call<std::string>("dumpJsonAt", path);
    const auto result = Poco::JSON::Parser().parse(json);
    _blockDesc = result.extract<Poco::JSON::Object::Ptr>();
}

void ProxyBlockEval::eval(const std::string &id)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");
    _proxyBlock = Pothos::Proxy(); //release old handle

    //load up the constructor args
    std::vector<Pothos::Proxy> ctorArgs;
    if (_blockDesc->isArray("args")) for (auto arg : *_blockDesc->getArray("args"))
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
    if (_blockDesc->isArray("calls")) for (auto call : *_blockDesc->getArray("calls"))
    {
        this->_handleCall(call.extract<Poco::JSON::Object::Ptr>());
    }
}

void ProxyBlockEval::handleCall(const std::string &callName)
{
    if (_blockDesc->isArray("calls")) for (auto call : *_blockDesc->getArray("calls"))
    {
        const auto callObj = call.extract<Poco::JSON::Object::Ptr>();
        if (callObj->get("name").extract<std::string>() == callName)
        {
            this->_handleCall(callObj);
        }
    }
}

void ProxyBlockEval::_handleCall(const Poco::JSON::Object::Ptr &callObj)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    const auto callName = callObj->get("name").extract<std::string>();
    std::vector<Pothos::Proxy> callArgs;
    for (auto arg : *callObj->getArray("args"))
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

Pothos::Object ProxyBlockEval::lookupOrEvalAsType(const Poco::Dynamic::Var &arg)
{
    //this is not an expression, but a native JSON type
    if (not arg.isString()) return _evalEnv->eval(arg.toString());

    //the expression is an already evaluated property
    const auto expr = arg.extract<std::string>();
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
