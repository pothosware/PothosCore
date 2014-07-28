// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "BlockEval.hpp"
#include <Pothos/Framework.hpp>

void BlockEval::eval(const std::string &id, const Poco::JSON::Object::Ptr &blockDesc)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");
    auto path = blockDesc->getValue<std::string>("path");

    //load up the constructor args
    std::vector<Pothos::Proxy> ctorArgs;
    for (auto arg : *blockDesc->getArray("args"))
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

    //inspect before making any calls -- calls may fails
    _portDesc = this->inspectPorts();

    //make the calls
    for (auto call : *blockDesc->getArray("calls"))
    {
        const auto callObj = call.extract<Poco::JSON::Object::Ptr>();
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

    //inspect after making calls -- ports may have changed
    _portDesc = this->inspectPorts();
}

static Poco::JSON::Array::Ptr portInfosToJSON(const std::vector<Pothos::PortInfo> &infos)
{
    Poco::JSON::Array::Ptr array = new Poco::JSON::Array();
    for (const auto &info : infos)
    {
        Poco::JSON::Object::Ptr portInfo = new Poco::JSON::Object();
        portInfo->set("name", info.name);
        portInfo->set("isSpecial", info.isSpecial);
        portInfo->set("size", info.dtype.size());
        portInfo->set("dtype", info.dtype.toString());
        array->add(portInfo);
    }
    return array;
}

Poco::JSON::Object::Ptr BlockEval::inspectPorts(void)
{
    auto block = _proxyBlock;
    Poco::JSON::Object::Ptr info = new Poco::JSON::Object();
    info->set("uid", block.call<std::string>("uid"));
    info->set("inputPorts", portInfosToJSON(block.call<std::vector<Pothos::PortInfo>>("inputPortInfo")));
    info->set("outputPorts", portInfosToJSON(block.call<std::vector<Pothos::PortInfo>>("outputPortInfo")));
    return info;
}

#include <Pothos/Managed.hpp>

static auto managedBlockEval = Pothos::ManagedClass()
    .registerConstructor<BlockEval, EvalEnvironment &>()
    .registerMethod(POTHOS_FCN_TUPLE(BlockEval, evalProperty))
    .registerMethod(POTHOS_FCN_TUPLE(BlockEval, eval))
    .registerMethod(POTHOS_FCN_TUPLE(BlockEval, getPortDesc))
    .registerMethod(POTHOS_FCN_TUPLE(BlockEval, getProxyBlock))
    .commit("Pothos/Util/BlockEval");
