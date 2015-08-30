// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "BlockEval.hpp"

void BlockEval::eval(const std::string &id, const Poco::JSON::Object::Ptr &blockDesc)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");
    auto path = blockDesc->getValue<std::string>("path");

    //load up the constructor args
    std::vector<Pothos::Proxy> ctorArgs;
    if (blockDesc->isArray("args")) for (auto arg : *blockDesc->getArray("args"))
    {
        const auto obj = this->lookupOrEvalAsType(arg);
        ctorArgs.push_back(env->convertObjectToProxy(obj));
    }

    //create the block
    try
    {
        _proxyBlock = registry.getHandle()->call(path, ctorArgs.data(), ctorArgs.size());
    }
    catch (const Pothos::Exception &ex)
    {
        throw Pothos::Exception("BlockEval factory("+path+")", ex);
    }
    _proxyBlock.callVoid("setName", id);

    //make the calls
    if (blockDesc->isArray("calls")) for (auto call : *blockDesc->getArray("calls"))
    {
        this->handleCall(call.extract<Poco::JSON::Object::Ptr>());
    }
}

void BlockEval::handleCall(const Poco::JSON::Object::Ptr &callObj)
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
        throw Pothos::Exception("BlockEval call("+callName+")", ex);
    }
}

Pothos::Object BlockEval::lookupOrEvalAsType(const Poco::Dynamic::Var &arg)
{
    //this is not an expression, but a native JSON type
    if (not arg.isString()) return _evalEnv->eval(arg.toString());

    //the expression is an already evaluated property
    const auto expr = arg.extract<std::string>();
    if (_properties.count(expr) != 0) return _properties.at(expr);

    //otherwise the expression must be evaluated
    //with the evaluated properties as global variables
    //use a new eval to avoid poisoning the globals
    auto evalEnv = EvalEnvironment::make();
    for (const auto &pair : _properties)
    {
        //Register can fail for non-primitive types
        //but those are not used in expressions anyway.
        try {evalEnv->registerConstantObj(pair.first, pair.second);}
        catch (...){}
    }
    return evalEnv->eval(expr);
}

#include <Pothos/Managed.hpp>

static auto managedBlockEval = Pothos::ManagedClass()
    .registerConstructor<BlockEval, const std::shared_ptr<EvalEnvironment> &>()
    .registerMethod(POTHOS_FCN_TUPLE(BlockEval, applyConstant))
    .registerMethod(POTHOS_FCN_TUPLE(BlockEval, removeConstant))
    .registerMethod(POTHOS_FCN_TUPLE(BlockEval, evalProperty))
    .registerMethod(POTHOS_FCN_TUPLE(BlockEval, setProperty))
    .registerMethod(POTHOS_FCN_TUPLE(BlockEval, eval))
    .registerMethod(POTHOS_FCN_TUPLE(BlockEval, handleCall))
    .registerMethod(POTHOS_FCN_TUPLE(BlockEval, getProxyBlock))
    .commit("Pothos/Util/BlockEval");
