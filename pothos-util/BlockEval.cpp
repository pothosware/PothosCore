// Copyright (c) 2014-2014 Josh Blum
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
        const auto propKey = arg.extract<std::string>();
        const auto obj = _properties[propKey];
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
        const auto propKey = arg.extract<std::string>();
        const auto obj = _properties[propKey];
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

#include <Pothos/Managed.hpp>

static auto managedBlockEval = Pothos::ManagedClass()
    .registerConstructor<BlockEval, EvalEnvironment &>()
    .registerMethod(POTHOS_FCN_TUPLE(BlockEval, evalProperty))
    .registerMethod(POTHOS_FCN_TUPLE(BlockEval, eval))
    .registerMethod(POTHOS_FCN_TUPLE(BlockEval, handleCall))
    .registerMethod(POTHOS_FCN_TUPLE(BlockEval, getProxyBlock))
    .commit("Pothos/Util/BlockEval");
